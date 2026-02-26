local mappings = require('voice_mappings')
local config = require('voice_config')
local echoPresets = require('voice_echo_presets')

local gVoiceVolume = mappings.DEFAULT_VOICE_VOLUME
local gVoiceEchoEnabled = (config.ECHO_ENABLED_DEFAULT == true)

local function set_voice_volume(_, value)
    gVoiceVolume = (tonumber(value) or 70) / 100.0
    if gVoiceVolume < 0.0 then gVoiceVolume = 0.0 end
    if gVoiceVolume > 1.0 then gVoiceVolume = 1.0 end
end

local function set_voice_echo_enabled(_, value)
    gVoiceEchoEnabled = (value == true)
end

if type(hook_mod_menu_slider) == 'function' then
    hook_mod_menu_slider('Voice Volume', math.floor(gVoiceVolume * 100), 0, 100, set_voice_volume)
end

if type(hook_mod_menu_checkbox) == 'function' then
    hook_mod_menu_checkbox('Echo Enabled', (config.ECHO_ENABLED_DEFAULT == true), set_voice_echo_enabled)
end

local pendingEchoes = {}

local gCachedLevelNum = nil
local gCachedAreaIndex = nil

local function get_current_level_and_area()
    if gNetworkPlayers == nil then return nil end
    if gNetworkPlayers[0] == nil then return nil end
    return gNetworkPlayers[0].currLevelNum, gNetworkPlayers[0].currAreaIndex
end

local function clear_pending_echoes()
    pendingEchoes = {}
end

local function refresh_cached_level_and_area(levelNum, areaIndex)
    if levelNum == 0 then levelNum = nil end
    if areaIndex == 0 then areaIndex = nil end
    if levelNum ~= nil then gCachedLevelNum = levelNum end
    if areaIndex ~= nil then gCachedAreaIndex = areaIndex end
end

local function on_level_init(_, levelNum, areaIdx, _, _)
    clear_pending_echoes()
    refresh_cached_level_and_area(levelNum, areaIdx)
end

local function on_warp(_, levelNum, areaIdx, _, _)
    clear_pending_echoes()
    refresh_cached_level_and_area(levelNum, areaIdx)
end

local function clone_preset(p)
    return {
        strength = p.strength,
        delayFrames = p.delayFrames,
        repeats = p.repeats,
        decay = p.decay,
        vanillaCurveExp = p.vanillaCurveExp,
    }
end

local function apply_override(dst, src)
    if src == nil then return end
    if src.strength ~= nil then dst.strength = src.strength end
    if src.delayFrames ~= nil then dst.delayFrames = src.delayFrames end
    if src.repeats ~= nil then dst.repeats = src.repeats end
    if src.decay ~= nil then dst.decay = src.decay end
    if src.vanillaCurveExp ~= nil then dst.vanillaCurveExp = src.vanillaCurveExp end
end

local function get_echo_preset(levelNum, areaIndex)
    local base = echoPresets.DEFAULT or {}
    local out = clone_preset(base)

    local presetKey = 'default'
    local lvl = nil
    if levelNum ~= nil and echoPresets.PRESETS ~= nil then
        lvl = echoPresets.PRESETS[levelNum]
        if lvl ~= nil then
            presetKey = levelNum
            apply_override(out, lvl)
        end
    end

    if areaIndex ~= nil and lvl ~= nil and type(lvl.areas) == 'table' then
        local area = lvl.areas[areaIndex]
        if area ~= nil then
            presetKey = tostring(levelNum) .. ':' .. tostring(areaIndex)
            apply_override(out, area)
        end
    end

    return out, presetKey
end

local function get_vanilla_echo_multiplier(levelNum, areaIndex, curveExp)
    if type(smlua_level_util_get_info) ~= 'function' then return 1.0 end
    if levelNum == nil or areaIndex == nil then return 1.0 end

    local info = smlua_level_util_get_info(levelNum)
    if info == nil then return 1.0 end

    local echo = 0
    if areaIndex == 1 then
        echo = info.echoLevel1 or 0
    elseif areaIndex == 2 then
        echo = info.echoLevel2 or 0
    elseif areaIndex == 3 then
        echo = info.echoLevel3 or 0
    end

    local mul = (tonumber(echo) or 0) / 255.0
    if mul < 0.0 then mul = 0.0 end
    if mul > 1.0 then mul = 1.0 end

    if curveExp == nil then curveExp = 1.0 end
    curveExp = tonumber(curveExp) or 1.0
    if curveExp < 0.0 then curveExp = 0.0 end

    if curveExp == 0.0 then
        mul = 1.0
    elseif curveExp == 1.0 then
        -- unchanged
    elseif curveExp == 2.0 then
        mul = mul * mul
    elseif curveExp == 3.0 then
        mul = mul * mul * mul
    else
        mul = mul ^ curveExp
    end
    return mul
end

local function queue_echo(sample, pos, volume)
    if not gVoiceEchoEnabled then return end
    if sample == nil or not sample.loaded then return end
    if type(get_global_timer) ~= 'function' then return end

    if pos == nil or pos.x == nil or pos.y == nil or pos.z == nil then return end

    local levelNum = gCachedLevelNum
    local areaIndex = gCachedAreaIndex
    if levelNum == nil or areaIndex == nil then
        levelNum, areaIndex = get_current_level_and_area()
    end

    if levelNum == 0 or areaIndex == 0 then
        return
    end
    refresh_cached_level_and_area(levelNum, areaIndex)

    local preset = get_echo_preset(levelNum, areaIndex)
    local strength = tonumber(preset.strength) or 0.0
    local delayFrames = tonumber(preset.delayFrames) or 5
    local repeats = tonumber(preset.repeats) or 0
    local decay = tonumber(preset.decay) or 0.65
    local curveExp = tonumber(preset.vanillaCurveExp) or 1.0

    if strength <= 0.0 then return end
    if repeats <= 0 then return end
    if delayFrames < 1 then delayFrames = 1 end
    if repeats > 4 then repeats = 4 end
    if delayFrames > 30 then delayFrames = 30 end
    if decay < 0.0 then decay = 0.0 end
    if decay > 1.0 then decay = 1.0 end

    local vanillaMul = get_vanilla_echo_multiplier(levelNum, areaIndex, curveExp)
    local v0 = volume * strength * vanillaMul * decay
    if v0 <= 0.0 then return end

    pendingEchoes[#pendingEchoes + 1] = {
        t = get_global_timer() + delayFrames,
        sample = sample,
        pos = { x = pos.x, y = pos.y, z = pos.z },
        volume = v0,
        repeatsLeft = repeats,
        delay = delayFrames,
        decay = decay,
    }
end

local function update_echoes()
    if #pendingEchoes == 0 then return end
    if type(get_global_timer) ~= 'function' then
        pendingEchoes = {}
        return
    end

    local now = get_global_timer()
    for i = #pendingEchoes, 1, -1 do
        local e = pendingEchoes[i]
        if now >= e.t then
            if e.sample ~= nil and e.sample.loaded and e.volume > 0.0 and e.pos ~= nil then
                if e.pos.x ~= nil and e.pos.y ~= nil and e.pos.z ~= nil then
                    audio_sample_play(e.sample, e.pos, e.volume)
                end
            end

            e.repeatsLeft = (e.repeatsLeft or 0) - 1
            if e.repeatsLeft > 0 then
                e.t = now + (e.delay or 4)
                e.volume = (e.volume or 0.0) * (e.decay or 0.65)
                if e.volume <= 0.0 then
                    table.remove(pendingEchoes, i)
                end
            else
                table.remove(pendingEchoes, i)
            end
        end
    end
end

local state = {
    [CT_MARIO] = { sample = nil, samplePath = nil, stream = nil, streamPath = nil, punchPool = {} },
    [CT_LUIGI] = { sample = nil, samplePath = nil, stream = nil, streamPath = nil, punchPool = {} },
    [CT_WARIO] = { sample = nil, samplePath = nil, stream = nil, streamPath = nil, punchPool = {} },
}

local function is_punch_sound(characterSound)
    return characterSound == CHAR_SOUND_PUNCH_HOO
        or characterSound == CHAR_SOUND_PUNCH_WAH
        or characterSound == CHAR_SOUND_PUNCH_YAH
        or characterSound == CHAR_SOUND_YAH_WAH_HOO
end

local function get_punch_sample(st, soundPath)
    if st == nil then return nil end
    if st.punchPool == nil then st.punchPool = {} end

    local entry = st.punchPool[soundPath]
    if entry == nil then
        entry = { samples = {}, nextIndex = 1 }
        st.punchPool[soundPath] = entry
    end

    local idx = entry.nextIndex or 1
    if idx < 1 then idx = 1 end
    local poolSize = tonumber(config.PUNCH_POOL_SIZE) or 3
    if poolSize < 1 then poolSize = 1 end
    if poolSize > 8 then poolSize = 8 end
    if idx > poolSize then idx = 1 end

    local s = entry.samples[idx]
    if s == nil or not s.loaded then
        s = audio_sample_load(soundPath)
        entry.samples[idx] = s
    end

    idx = idx + 1
    if idx > poolSize then idx = 1 end
    entry.nextIndex = idx

    return s
end

local function choose_voice(voice)
    if type(voice) == 'table' then
        if #voice == 0 then return nil end
        return voice[math.random(#voice)]
    end
    return voice
end

local function resolve_path(characterType, filename)
    if filename == nil then return nil end
    if type(filename) ~= 'string' then return filename end
    if filename:find('/', 1, true) then
        return filename
    end

    local dir = mappings.CHARACTER_SOUND_DIR[characterType]
    local legacy = 'sound/' .. filename
    local legacy2 = 'audio/' .. filename

    if dir == nil then
        return legacy2
    end

    local p = 'audio/' .. dir .. '/' .. filename

    if type(mod_file_exists) == 'function' then
        if mod_file_exists(p) then
            return p
        end
        if mod_file_exists(legacy) then
            return legacy
        end
        if mod_file_exists(legacy2) then
            return legacy2
        end
    end

    return p
end

local function stop_sample(st, soundPath)
    local voice_sample = st.sample
    if voice_sample == nil or not voice_sample.loaded then
        return nil
    end

    audio_sample_stop(voice_sample)
    if st.samplePath == soundPath then
        return voice_sample
    end

    audio_sample_destroy(voice_sample)
    st.sample = nil
    st.samplePath = nil
    return nil
end

local function stop_stream(st)
    if st.stream ~= nil then
        audio_stream_stop(st.stream)
        audio_stream_destroy(st.stream)
        st.stream = nil
        st.streamPath = nil
    end
end

local function play_voice_for_character(m, characterType, characterSound, voice)
    local st = state[characterType]
    if st == nil then return nil end

    local chosen = choose_voice(voice)
    if chosen == nil then return nil end

    local soundPath = resolve_path(characterType, chosen)
    if soundPath == nil then return nil end

    if type(soundPath) ~= 'string' then
        return soundPath
    end

    local isPunch = is_punch_sound(characterSound)
    local voice_sample = nil
    if not isPunch then
        voice_sample = stop_sample(st, soundPath)
    end

    if (m.area == nil or m.area.camera == nil) then
        if st.streamPath ~= soundPath then
            stop_stream(st)
            st.stream = audio_stream_load(soundPath)
            st.streamPath = soundPath
        end
        if st.stream == nil then
            return nil
        end
        audio_stream_play(st.stream, true, gVoiceVolume)
        return 0
    end

    if voice_sample == nil then
        if isPunch then
            voice_sample = get_punch_sample(st, soundPath)
        else
            voice_sample = audio_sample_load(soundPath)
        end
    end

    if voice_sample == nil or not voice_sample.loaded then
        return nil
    end

    audio_sample_play(voice_sample, m.pos, gVoiceVolume)
    queue_echo(voice_sample, m.pos, gVoiceVolume)
    if not isPunch then
        st.sample = voice_sample
        st.samplePath = soundPath
    end
    return 0
end

local function on_character_sound(m, characterSound)
    if m == nil or m.character == nil then return nil end

    local characterType = m.character.type
    local voiceTable = mappings.VOICE_TABLES[characterType]
    if voiceTable == nil then
        return nil
    end

    if characterSound == CHAR_SOUND_HAHA and m.hurtCounter > 0 then
        return nil
    end

    local voice = voiceTable[characterSound]
    if voice == nil then
        return nil
    end

    return play_voice_for_character(m, characterType, characterSound, voice)
end

hook_event(HOOK_CHARACTER_SOUND, on_character_sound)
hook_event(HOOK_UPDATE, update_echoes)
hook_event(HOOK_ON_LEVEL_INIT, on_level_init)
hook_event(HOOK_ON_WARP, on_warp)

return {
    set_voice_volume = set_voice_volume,
}
