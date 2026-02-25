-- name: Streamed Music Playback
-- description: Replaces vanilla sequence music with streamed audio tracks per level/area, with loop points and vanilla-like fading.

local okTracks, tracksOrErr = pcall(require, 'tracks')
local tracks = okTracks and tracksOrErr or nil

local okBossIntro, bossIntroOrErr = pcall(require, 'boss_intro')
local boss_intro = okBossIntro and bossIntroOrErr or nil

local sConsoleOk = (type(log_to_console) == 'function')
local function log_warn(msg)
    print("[streamed-music] WARNING: " .. msg)
    if sConsoleOk then
        log_to_console("[streamed-music] " .. msg, CONSOLE_MESSAGE_WARNING)
    end
end

if not okTracks then
    log_warn("Failed to load tracks.lua: " .. tostring(tracksOrErr))
end

local state = {
    enabled = true,
    bg = {
        stream = nil,
        path = nil,
        fadeOutStream = nil,
        fadeOutPath = nil,
        phaseFadeFrames = 30,
        phaseFadeTimer = 0,
        baseVolume = 1.0,
        currentVolume = 0.0,
    },
    env = {
        stream = nil,
        path = nil,
        seqId = nil,
        baseVolume = 1.0,
        currentVolume = 0.0,
        noSeqGraceFrames = 0,
        oneShotDebounceFrames = 0,
        savedSecondarySeqId = nil,
        savedSecondaryPath = nil,
        savedSecondaryPos = nil,
        wasInOverlay = false,
        overlayHoldFrames = 0,
        overlayHoldSeqId = nil,
    },
    seq = {
        noSeqGraceFrames = 0,
        forcedSeqId = nil,
        lastSeqId = nil,
        lastBgSeqId = nil,
        lastMissingSeqId = nil,
        lastDynamicIndex = nil,
    },
    boss = {
        introController = nil,
        defeatedLatch = false,
        cutsceneLock = false,
        cutsceneSawStarSpawn = false,
    },
    flags = {
        muteEnvPlayer = false,
        pausedByGame = false,
        forceFadeOut = false,
        silentFrames = 0,
        isPausedForFade = false,
        duckBgForEnvOverlay = false,
    },
    mutedSeqIds = {},
    frameCounter = 0,
}

if boss_intro ~= nil and type(boss_intro.new) == 'function' then
    state.boss.introController = boss_intro.new({
        mod_storage_exists = mod_storage_exists,
        mod_storage_load_bool = mod_storage_load_bool,
        mod_storage_save_bool = mod_storage_save_bool,
        hook_mod_menu_checkbox = hook_mod_menu_checkbox,
        get_dialog_box_state = get_dialog_box_state,
        get_dialog_id = get_dialog_id,
    })
end

-- These sequences are used for short jingles/cutscenes and should not force
-- the level music to restart when they begin/end.
local function is_non_restart_overlay_seq(seqId)
    return seqId == SEQ_EVENT_CUTSCENE_COLLECT_STAR
        or seqId == SEQ_EVENT_HIGH_SCORE
        or seqId == SEQ_EVENT_SOLVE_PUZZLE
        or seqId == SEQ_EVENT_TOAD_MESSAGE
        or seqId == SEQ_EVENT_PEACH_MESSAGE
        or seqId == SEQ_EVENT_CUTSCENE_STAR_SPAWN
        or seqId == SEQ_EVENT_CUTSCENE_COLLECT_KEY
end

local function is_cap_seq(seqId)
    return seqId == SEQ_EVENT_POWERUP
        or seqId == SEQ_EVENT_METAL_CAP
end

-- C returns u16(-1) when there is no queued background music.
-- Some states also report 0.
local function seq_args_is_none(seqArgs)
    return seqArgs == nil or seqArgs == 0 or seqArgs == -1 or seqArgs == 0xFFFF or seqArgs == 65535
end

local function seq_id_from_args(seqArgs)
    if seqArgs == nil then return nil end
    if seq_args_is_none(seqArgs) then return nil end
    if bit32 ~= nil and bit32.band ~= nil then
        -- low byte contains the sequence id, possibly OR'd with SEQ_VARIATION (0x80)
        return bit32.band(seqArgs, 0x7F)
    end
    -- fallback (Lua 5.3+)
    if type(seqArgs) == "number" then
        return seqArgs & 0x7F
    end
    return nil
end

local function is_photo_mode_active()
    if gLakituState ~= nil and type(gLakituState) == 'table' and gLakituState.mode ~= nil then
        return gLakituState.mode == CAMERA_MODE_PHOTO_MODE
    end
    return false
end

local function should_pause_stream_for_game_state()
    if type(is_game_paused) == 'function' and is_game_paused() then
        -- gMenuMode is used for multiple in-game menu states.
        -- Course-complete (Save & Continue) uses menu modes 2/3; vanilla music continues,
        -- so don't hard-pause streamed music there.
        if type(get_menu_mode) == 'function' then
            local mm = get_menu_mode()
            if mm == 2 or mm == 3 then
                return false
            end
        end
        return true
    end

    if type(is_djui_player_menu_open) == 'function' and is_djui_player_menu_open() then
        return true
    end

    if type(is_play_mode_photo_mode) == 'function' and is_play_mode_photo_mode() then
        return true
    end

    if type(is_camera_photo_mode) == 'function' and is_camera_photo_mode() then
        return true
    end

    return false
end

local function clamp01(x)
    if x < 0 then return 0 end
    if x > 1 then return 1 end
    return x
end

local function get_track_for_seq_id(seqId)
    if seqId == nil then return nil end
    if tracks == nil then return nil end
    return tracks[seqId]
end

local function get_phase_track(baseTrack, dynIndex)
    if baseTrack == nil then return nil end
    if type(baseTrack) ~= 'table' then return baseTrack end
    if baseTrack.phases == nil or type(baseTrack.phases) ~= 'table' then return baseTrack end
    if dynIndex == nil then return baseTrack end

    local phase = tonumber(dynIndex) or 0
    if phase < 0 then phase = 0 end
    -- Vanilla uses 0xff when no dynamics apply.
    if phase == 255 then
        return baseTrack
    end

    -- Map dynamic index to 1-based phase table (clamped)
    local idx = phase + 1
    if idx < 1 then idx = 1 end
    if idx > #baseTrack.phases then idx = #baseTrack.phases end
    return baseTrack.phases[idx] or baseTrack
end

local function stop_stream()
    if state.bg.stream ~= nil then
        audio_stream_stop(state.bg.stream)
        audio_stream_destroy(state.bg.stream)
        state.bg.stream = nil
        state.bg.path = nil
    end
    if state.bg.fadeOutStream ~= nil then
        audio_stream_stop(state.bg.fadeOutStream)
        audio_stream_destroy(state.bg.fadeOutStream)
        state.bg.fadeOutStream = nil
        state.bg.fadeOutPath = nil
    end
    state.bg.phaseFadeTimer = 0
    if state.env.stream ~= nil then
        audio_stream_stop(state.env.stream)
        audio_stream_destroy(state.env.stream)
        state.env.stream = nil
        state.env.path = nil
    end
    state.env.seqId = nil
    state.env.baseVolume = 1.0
    state.env.currentVolume = 0.0
end

local function stop_bg_stream_only()
    if state.bg.stream ~= nil then
        audio_stream_stop(state.bg.stream)
        audio_stream_destroy(state.bg.stream)
        state.bg.stream = nil
        state.bg.path = nil
    end
    if state.bg.fadeOutStream ~= nil then
        audio_stream_stop(state.bg.fadeOutStream)
        audio_stream_destroy(state.bg.fadeOutStream)
        state.bg.fadeOutStream = nil
        state.bg.fadeOutPath = nil
    end
    state.bg.phaseFadeTimer = 0
end

local function configure_stream_looping(stream, track)
    if stream == nil or track == nil then return end

    if track.loopStart ~= nil and track.loopEnd ~= nil and track.loopStart > 1 then
        audio_stream_set_looping(stream, true)
        audio_stream_set_loop_points(stream, track.loopStart, track.loopEnd)
    else
        audio_stream_set_looping(stream, track.loop == true)
    end
end

local function begin_phase_crossfade(newTrack)
    if state.bg.stream == nil or newTrack == nil or newTrack.file == nil then return false end
    if state.bg.path == newTrack.file then return false end

    -- If we are already mid-crossfade and another rapid phase change occurs,
    -- cancel the in-progress crossfade so we don't end up with both streams
    -- effectively muted (e.g. toggling phases quickly back-and-forth).
    if state.bg.fadeOutStream ~= nil and state.bg.phaseFadeTimer > 0 then
        audio_stream_stop(state.bg.fadeOutStream)
        audio_stream_destroy(state.bg.fadeOutStream)
        state.bg.fadeOutStream = nil
        state.bg.fadeOutPath = nil
        state.bg.phaseFadeTimer = 0
    end

    local pos = 0.0
    if type(audio_stream_get_position) == 'function' then
        pos = audio_stream_get_position(state.bg.stream) or 0.0
    end

    local newStream = audio_stream_load(newTrack.file)
    if newStream == nil then
        log_warn("audio_stream_load failed for '" .. tostring(newTrack.file) .. "'")
        return false
    end

    configure_stream_looping(newStream, newTrack)

    -- Start new stream muted, seek to current position, then fade in.
    audio_stream_play(newStream, true, 1.0)
    if type(audio_stream_set_position) == 'function' then
        audio_stream_set_position(newStream, pos)
    end
    audio_stream_set_volume(newStream, 0.0)

    -- Move current stream to fade-out slot.
    if state.bg.fadeOutStream ~= nil then
        audio_stream_stop(state.bg.fadeOutStream)
        audio_stream_destroy(state.bg.fadeOutStream)
        state.bg.fadeOutStream = nil
        state.bg.fadeOutPath = nil
    end
    state.bg.fadeOutStream = state.bg.stream
    state.bg.fadeOutPath = state.bg.path

    state.bg.stream = newStream
    state.bg.path = newTrack.file
    state.bg.baseVolume = newTrack.volume or 1.0

    state.bg.phaseFadeTimer = state.bg.phaseFadeFrames
    return true
end

local function ensure_bg_stream(track)
    if track == nil or track.file == nil then
        return
    end

    if state.bg.stream ~= nil and state.bg.path == track.file then
        return
    end

    local newStream = audio_stream_load(track.file)
    if newStream == nil then
        log_warn("audio_stream_load failed for '" .. tostring(track.file) .. "'")
        return
    end

    if state.bg.stream ~= nil then
        audio_stream_stop(state.bg.stream)
        audio_stream_destroy(state.bg.stream)
    end
    if state.bg.fadeOutStream ~= nil then
        audio_stream_stop(state.bg.fadeOutStream)
        audio_stream_destroy(state.bg.fadeOutStream)
        state.bg.fadeOutStream = nil
        state.bg.fadeOutPath = nil
    end
    state.bg.phaseFadeTimer = 0

    state.bg.stream = newStream
    state.bg.path = track.file

    configure_stream_looping(state.bg.stream, track)

    state.bg.baseVolume = track.volume or 1.0
    state.bg.currentVolume = 0.0
    audio_stream_play(state.bg.stream, true, 1.0)
end

local function ensure_env_stream(seqId, track)
    if track == nil or track.file == nil then
        return
    end

    local isOneShot = true
    if track.loop == true then
        isOneShot = false
    end
    if track.loopStart ~= nil and track.loopEnd ~= nil and track.loopStart > 1 and track.loopEnd > 0 then
        isOneShot = false
    end

    -- One-shot jingles can be re-requested rapidly during cutscene transitions.
    -- If we're already on the same file, don't restart it.
    if isOneShot and state.env.stream ~= nil and state.env.path == track.file and state.env.oneShotDebounceFrames > 0 then
        state.env.seqId = seqId
        return
    end

    if state.env.seqId ~= nil and state.env.seqId == seqId and state.env.stream ~= nil and state.env.path == track.file then
        return
    end

    local newStream = audio_stream_load(track.file)
    if newStream == nil then
        log_warn("audio_stream_load failed for env '" .. tostring(track.file) .. "'")
        return
    end

    if state.env.stream ~= nil then
        audio_stream_stop(state.env.stream)
        audio_stream_destroy(state.env.stream)
    end

    state.env.stream = newStream
    state.env.path = track.file
    state.env.seqId = seqId
    state.env.baseVolume = track.volume or 1.0
    state.env.currentVolume = 0.0
    if isOneShot then
        state.env.oneShotDebounceFrames = 45
    else
        state.env.oneShotDebounceFrames = 0
    end

    if track.loopStart ~= nil and track.loopEnd ~= nil and track.loopStart > 1 and track.loopEnd > 0 then
        audio_stream_set_looping(state.env.stream, true)
        audio_stream_set_loop_points(state.env.stream, track.loopStart, track.loopEnd)
    else
        audio_stream_set_looping(state.env.stream, track.loop == true)
    end

    audio_stream_play(state.env.stream, true, 1.0)
end

local function mute_vanilla_level_music()
    local seqArgs = get_current_background_music()
    if seq_args_is_none(seqArgs) then return end

    local seqId = seq_id_from_args(seqArgs)
    if seqId == nil then return end
    if is_non_restart_overlay_seq(seqId) then return end
    -- Do not mutate the sequence's default volume to mute MIDI.
    -- The engine's set_sequence_player_volume() clamps SEQ_PLAYER_LEVEL.fadeVolume
    -- to the default volume each frame; setting it to 0 would also zero our
    -- authoritative fade signal and would mute streamed playback.
    return
end

local function unmute_all_sequences()
    state.mutedSeqIds = {}
end

local function try_set_stream_volume(stream, vol)
    if stream == nil then return true end
    if type(audio_stream_set_volume) ~= 'function' then return true end
    local ok = pcall(audio_stream_set_volume, stream, vol)
    return ok
end

local function update_stream_volume()
    if state.bg.stream == nil and state.env.stream == nil then return end

    -- Always enforce vanilla MIDI mute/unmute, even if we early-return due to pause.
    if type(set_sequence_player_volume_override) == 'function' then
        set_sequence_player_volume_override(SEQ_PLAYER_LEVEL, (state.bg.stream ~= nil), 0.0)
        set_sequence_player_volume_override(SEQ_PLAYER_ENV, (state.env.stream ~= nil), 0.0)
    elseif type(set_sequence_player_volume) == 'function' then
        -- Fallback if override isn't available.
        set_sequence_player_volume(SEQ_PLAYER_LEVEL, (state.bg.stream ~= nil) and 0.0 or 1.0)
        set_sequence_player_volume(SEQ_PLAYER_ENV, (state.env.stream ~= nil) and 0.0 or 1.0)
    end

    if should_pause_stream_for_game_state() then
        if not state.flags.pausedByGame then
            if state.bg.stream ~= nil then audio_stream_pause(state.bg.stream) end
            if state.bg.fadeOutStream ~= nil then audio_stream_pause(state.bg.fadeOutStream) end
            if state.env.stream ~= nil then audio_stream_pause(state.env.stream) end
            state.flags.pausedByGame = true
        end

        -- Hard-mute while paused/photo mode to guarantee silence even if pause is imperfect.
        if not try_set_stream_volume(state.bg.stream, 0.0) then
            state.bg.stream = nil
            state.bg.path = nil
        end
        if not try_set_stream_volume(state.bg.fadeOutStream, 0.0) then
            state.bg.fadeOutStream = nil
            state.bg.fadeOutPath = nil
            state.bg.phaseFadeTimer = 0
        end
        if not try_set_stream_volume(state.env.stream, 0.0) then
            state.env.stream = nil
            state.env.path = nil
            state.env.seqId = nil
            state.env.baseVolume = 1.0
            state.env.currentVolume = 0.0
        end
        return
    elseif state.flags.pausedByGame then
        if state.bg.stream ~= nil then audio_stream_play(state.bg.stream, false, 1.0) end
        if state.bg.fadeOutStream ~= nil then audio_stream_play(state.bg.fadeOutStream, false, 1.0) end
        if state.env.stream ~= nil then audio_stream_play(state.env.stream, false, 1.0) end
        state.flags.pausedByGame = false
    end

    -- Use the level sequence player's fadeVolume as the authoritative fade signal.
    local fadeVol = 0.0
    if type(get_sequence_player_fade_volume) == 'function' then
        fadeVol = get_sequence_player_fade_volume(SEQ_PLAYER_LEVEL) or 0.0
    end

    local maxVol = 1.0
    if type(get_current_background_music_default_volume) == 'function' then
        maxVol = (get_current_background_music_default_volume() or 127) / 127.0
        if maxVol <= 0 then maxVol = 1.0 end
    end

    -- Convert absolute fadeVolume (0..maxVol) into a 0..1 multiplier.
    local ratio = clamp01(fadeVol / maxVol)

    local desired = state.bg.baseVolume * ratio
    if state.flags.forceFadeOut then
        desired = 0.0
    end
    if state.flags.duckBgForEnvOverlay then
        desired = desired * 0.35
    end
    -- Let vanilla define fade timing; don't add additional smoothing here.
    state.bg.currentVolume = desired
    local bgVol = state.bg.currentVolume
    local bgFadeVol = 0.0
    if state.bg.fadeOutStream ~= nil and state.bg.phaseFadeTimer > 0 then
        local t = (state.bg.phaseFadeFrames - state.bg.phaseFadeTimer) / state.bg.phaseFadeFrames
        if t < 0 then t = 0 end
        if t > 1 then t = 1 end
        bgVol = state.bg.currentVolume * t
        bgFadeVol = state.bg.currentVolume * (1.0 - t)

        state.bg.phaseFadeTimer = state.bg.phaseFadeTimer - 1
        if state.bg.phaseFadeTimer <= 0 then
            audio_stream_stop(state.bg.fadeOutStream)
            audio_stream_destroy(state.bg.fadeOutStream)
            state.bg.fadeOutStream = nil
            state.bg.fadeOutPath = nil
            state.bg.phaseFadeTimer = 0
        end
    end

    if not try_set_stream_volume(state.bg.stream, bgVol) then
        state.bg.stream = nil
        state.bg.path = nil
    end
    if not try_set_stream_volume(state.bg.fadeOutStream, bgFadeVol) then
        state.bg.fadeOutStream = nil
        state.bg.fadeOutPath = nil
        state.bg.phaseFadeTimer = 0
    end

    if not try_set_stream_volume(state.env.stream, state.env.currentVolume) then
        state.env.stream = nil
        state.env.path = nil
        state.env.seqId = nil
        state.env.baseVolume = 1.0
        state.env.currentVolume = 0.0
    end

    -- Vanilla does not pause the sequence player when it fades to 0; it simply runs silent.
    -- Keep streamed audio running as well to match restart semantics.
end

local function refresh_track()
    if state.seq.forcedSeqId ~= nil then
        local track = get_track_for_seq_id(state.seq.forcedSeqId)
        if track ~= nil then
            state.seq.noSeqGraceFrames = 0
            ensure_bg_stream(track)
            return
        end
    end

    local seqArgs = get_current_background_music()
    if seq_args_is_none(seqArgs) then
        -- Goddard/title demo reel can briefly report no queued background music
        -- while it swaps showcased levels. If we're currently playing the title
        -- or credits track, keep the stream alive to avoid restarting.
        local titleTrack = get_track_for_seq_id(SEQ_MENU_TITLE_SCREEN)
        local creditsTrack = get_track_for_seq_id(SEQ_EVENT_CUTSCENE_CREDITS)
        local isTitleCreditsStream = false
        if state.bg.path ~= nil then
            if titleTrack ~= nil and titleTrack.file ~= nil and state.bg.path == titleTrack.file then
                isTitleCreditsStream = true
            elseif creditsTrack ~= nil and creditsTrack.file ~= nil and state.bg.path == creditsTrack.file then
                isTitleCreditsStream = true
            end
        end

        if state.bg.stream ~= nil and (isTitleCreditsStream or state.seq.lastBgSeqId == SEQ_MENU_TITLE_SCREEN or state.seq.lastBgSeqId == SEQ_EVENT_CUTSCENE_CREDITS) then
            state.seq.noSeqGraceFrames = 0
            return
        end
        -- grace period to avoid menu transition restarts
        -- Don't kill the stream while we have a forced seq selection.
        state.seq.noSeqGraceFrames = state.seq.noSeqGraceFrames + 1
        if state.seq.noSeqGraceFrames >= 30 then
            stop_bg_stream_only()
            state.seq.noSeqGraceFrames = 0
        end
        return
    end

    state.seq.noSeqGraceFrames = 0

    local seqId = seq_id_from_args(seqArgs)

    -- Vanilla restarts the base level music from the beginning when cap music ends.
    -- Force a hard restart when transitioning from cap -> non-cap.
    if state.seq.lastSeqId ~= nil and is_cap_seq(state.seq.lastSeqId) and (not is_cap_seq(seqId)) then
        stop_bg_stream_only()
    end

    if is_non_restart_overlay_seq(seqId) and state.seq.lastBgSeqId ~= nil and not state.boss.defeatedLatch then
        seqId = state.seq.lastBgSeqId
    end

    if state.boss.defeatedLatch then
        if seqId == SEQ_LEVEL_BOSS_KOOPA or seqId == SEQ_LEVEL_BOSS_KOOPA_FINAL or seqId == SEQ_EVENT_BOSS then
            return
        end
    end

    local track = get_track_for_seq_id(seqId)
    if track == nil then
        if seqId ~= nil and state.seq.lastMissingSeqId ~= seqId then
            state.seq.lastMissingSeqId = seqId
            log_warn("No track mapping for seqId " .. tostring(seqId))
        end
        -- If this is an overlay sequence with no mapping, don't kill the
        -- currently-playing background stream.
        if not is_non_restart_overlay_seq(seq_id_from_args(seqArgs)) then
            stop_stream()
        end
        return
    end

    local dynIndex = nil
    if type(get_current_music_dynamic) == 'function' then
        dynIndex = get_current_music_dynamic()
    end
    local phaseTrack = get_phase_track(track, dynIndex)
    if phaseTrack ~= nil and type(track) == 'table' and track.phases ~= nil then
        if not begin_phase_crossfade(phaseTrack) then
            ensure_bg_stream(phaseTrack)
        end
    else
        ensure_bg_stream(phaseTrack)
    end
end

local function on_level_init()
    if not state.enabled then return end

    state.seq.forcedSeqId = nil
    state.seq.noSeqGraceFrames = 0
    state.flags.forceFadeOut = false
    state.boss.defeatedLatch = false
    state.boss.cutsceneLock = false
    state.boss.cutsceneSawStarSpawn = false

    -- The title screen demo reel can trigger level init events while the
    -- title/credits music should continue uninterrupted.
    local seqArgs = get_current_background_music()
    local seqId = seq_id_from_args(seqArgs)
    local titleTrack = get_track_for_seq_id(SEQ_MENU_TITLE_SCREEN)
    local creditsTrack = get_track_for_seq_id(SEQ_EVENT_CUTSCENE_CREDITS)
    local isTitleCredits = (seqId == SEQ_MENU_TITLE_SCREEN or seqId == SEQ_EVENT_CUTSCENE_CREDITS)
    local isTitleCreditsStream = false
    if state.bg.path ~= nil then
        if titleTrack ~= nil and titleTrack.file ~= nil and state.bg.path == titleTrack.file then
            isTitleCreditsStream = true
        elseif creditsTrack ~= nil and creditsTrack.file ~= nil and state.bg.path == creditsTrack.file then
            isTitleCreditsStream = true
        end
    end

    if not (isTitleCredits and isTitleCreditsStream) then
        stop_stream()
    end
end

local function on_warp()
    if not state.enabled then return end

    state.seq.forcedSeqId = nil
    state.seq.noSeqGraceFrames = 0
    state.flags.forceFadeOut = false
    state.boss.defeatedLatch = false
    state.boss.cutsceneLock = false
    state.boss.cutsceneSawStarSpawn = false

    -- Many in-level warps (pipes, teleports, instant warps) should not restart
    -- the current background music. Don't destroy streams here.
    -- Instead, clear cached IDs so we re-sync cleanly after the warp.
    state.seq.lastSeqId = nil
    state.seq.lastBgSeqId = nil
    state.seq.lastMissingSeqId = nil
end

local function on_instant_warp()
    if not state.enabled then return end
    mute_vanilla_level_music()
    refresh_track()
end

local function on_clear_areas()
    if not state.enabled then return end
    -- Don't hard-stop the stream here; the engine can clear areas during the
    -- title screen demo reel and we don't want the title track to restart.
    -- Instead, force a refresh on the next update.
    state.seq.lastSeqId = nil
    state.seq.lastBgSeqId = nil
end

local function on_update()
    if not state.enabled then return end

    mute_vanilla_level_music()

    -- During pause/photo mode, avoid loading/restarting streams; just keep them muted/paused.
    if should_pause_stream_for_game_state() then
        update_stream_volume()
        return
    end

    state.frameCounter = state.frameCounter + 1
    if state.env.oneShotDebounceFrames > 0 then
        state.env.oneShotDebounceFrames = state.env.oneShotDebounceFrames - 1
        if state.env.oneShotDebounceFrames < 0 then state.env.oneShotDebounceFrames = 0 end
    end

    local seqArgs = get_current_background_music()
    local reportedSeqId = seq_id_from_args(seqArgs)

    local dynIndex = nil
    if type(get_current_music_dynamic) == 'function' then
        dynIndex = get_current_music_dynamic()
    end

    -- Vanilla secondary music state is authoritative for BBH Merry-Go-Round.
    local secondarySeqId = nil
    local secondaryVol = 0
    if type(get_current_secondary_music_seq_id) == 'function' then
        secondarySeqId = get_current_secondary_music_seq_id()
    end
    if type(get_current_secondary_music_volume) == 'function' then
        secondaryVol = get_current_secondary_music_volume() or 0
    end

    if secondarySeqId ~= nil then
        -- When secondary music is active (e.g. BBH merry-go-round), vanilla will still
        -- play short overlay jingles on the ENV sequence player.
        -- If we always force secondarySeqId here, those jingles will never be heard.
        local overlayEnvSeqId = nil
        if type(get_env_sequence_id) == 'function' then
            overlayEnvSeqId = get_env_sequence_id()
        end
        if overlayEnvSeqId == 0 then overlayEnvSeqId = nil end
        if overlayEnvSeqId ~= nil and bit32 ~= nil and bit32.band ~= nil then
            overlayEnvSeqId = bit32.band(overlayEnvSeqId, 0x7F)
        end

        local function overlay_hold_duration(seqId)
            if seqId == SEQ_EVENT_SOLVE_PUZZLE then
                return 90
            end
            if seqId == SEQ_EVENT_CUTSCENE_STAR_SPAWN then
                return 150
            end
            if seqId == SEQ_EVENT_CUTSCENE_COLLECT_STAR then
                return 240
            end
            if seqId == SEQ_EVENT_HIGH_SCORE then
                return 210
            end
            if seqId == SEQ_EVENT_CUTSCENE_COLLECT_KEY then
                return 240
            end
            return 120
        end

        local overlayJustDetected = (overlayEnvSeqId ~= nil and is_non_restart_overlay_seq(overlayEnvSeqId))
        if overlayJustDetected then
            state.env.overlayHoldSeqId = overlayEnvSeqId
            state.env.overlayHoldFrames = overlay_hold_duration(overlayEnvSeqId)
        elseif state.env.overlayHoldFrames > 0 then
            state.env.overlayHoldFrames = state.env.overlayHoldFrames - 1
            if state.env.overlayHoldFrames <= 0 then
                state.env.overlayHoldFrames = 0
                state.env.overlayHoldSeqId = nil
            end
        end

        local shouldUseOverlay = overlayJustDetected or (state.env.overlayHoldFrames > 0 and state.env.overlayHoldSeqId ~= nil)
        state.flags.duckBgForEnvOverlay = shouldUseOverlay

        -- If we are about to switch away from the secondary track to an overlay jingle,
        -- save the current secondary stream position so we can resume without restarting.
        if shouldUseOverlay then
            if not state.env.wasInOverlay and state.env.stream ~= nil and state.env.seqId == secondarySeqId then
                local secondaryTrack = get_track_for_seq_id(secondarySeqId)
                if secondaryTrack ~= nil and secondaryTrack.file ~= nil then
                    state.env.savedSecondarySeqId = secondarySeqId
                    state.env.savedSecondaryPath = secondaryTrack.file
                    if type(audio_stream_get_position) == 'function' then
                        state.env.savedSecondaryPos = audio_stream_get_position(state.env.stream)
                    else
                        state.env.savedSecondaryPos = nil
                    end
                end
            end
            state.env.wasInOverlay = true
        else
            -- Overlay ended; resume secondary music from saved position.
            if state.env.wasInOverlay and state.env.savedSecondarySeqId == secondarySeqId then
                local secondaryTrack = get_track_for_seq_id(secondarySeqId)
                if secondaryTrack ~= nil and secondaryTrack.file ~= nil then
                    ensure_env_stream(secondarySeqId, secondaryTrack)
                    if state.env.stream ~= nil and state.env.savedSecondaryPos ~= nil and type(audio_stream_set_position) == 'function' then
                        audio_stream_set_position(state.env.stream, state.env.savedSecondaryPos)
                    end
                end
            end
            state.env.wasInOverlay = false
        end

        local chosenOverlaySeqId = overlayJustDetected and overlayEnvSeqId or state.env.overlayHoldSeqId
        local chosenSeqId = shouldUseOverlay and chosenOverlaySeqId or secondarySeqId

        local envTrack = get_track_for_seq_id(chosenSeqId)
        if envTrack ~= nil then
            ensure_env_stream(chosenSeqId, envTrack)

            local v = 1.0
            if shouldUseOverlay then
                -- Overlay jingles don't have a secondary-volume scalar.
                v = 1.0
            else
                -- Vanilla uses 0-127 volumes for secondary music.
                v = (secondaryVol / 127.0)
                if v < 0 then v = 0 end
                if v > 1 then v = 1 end
            end

            local desiredEnv = state.env.baseVolume * v
            if shouldUseOverlay then
                state.env.currentVolume = desiredEnv
            else
                state.env.currentVolume = state.env.currentVolume + (desiredEnv - state.env.currentVolume) * 0.12
            end

            -- Always mute vanilla ENV when secondary music/overlays are active and mapped.
            if type(set_sequence_player_volume) == 'function' then
                set_sequence_player_volume(SEQ_PLAYER_ENV, 0.0)
            end
            if type(fade_out_sequence_player) == 'function' then
                fade_out_sequence_player(SEQ_PLAYER_ENV, 0)
            end
        else
            if state.env.stream ~= nil then
                audio_stream_stop(state.env.stream)
                audio_stream_destroy(state.env.stream)
                state.env.stream = nil
                state.env.path = nil
                state.env.seqId = nil
                state.env.baseVolume = 1.0
                state.env.currentVolume = 0.0
            end
        end
    else
        state.flags.duckBgForEnvOverlay = false
        -- Not all SEQ_PLAYER_ENV sequences go through play_secondary_music().
        -- Star spawn / course-clear fanfare are started directly on the ENV player,
        -- so fall back to reading the ENV player's active seq id.
        local envSeqId = nil
        if type(get_env_sequence_id) == 'function' then
            envSeqId = get_env_sequence_id()
        end

        if envSeqId == 0 then
            envSeqId = nil
        end

        -- ENV seq ids may include the variation bit; normalize to base id.
        if envSeqId ~= nil and bit32 ~= nil and bit32.band ~= nil then
            envSeqId = bit32.band(envSeqId, 0x7F)
        end

        local envTrack = nil
        if envSeqId ~= nil then
            envTrack = get_track_for_seq_id(envSeqId)
        end

        if envTrack ~= nil then
            state.env.noSeqGraceFrames = 0
            ensure_env_stream(envSeqId, envTrack)

            -- For ENV sequences that don't use play_secondary_music() (like star
            -- spawn/course-clear fanfare), there's no secondary-volume global to
            -- follow. Treat streamed ENV as full-volume and rely on the existing
            -- volumeScale override in update_stream_volume() to mute vanilla ENV.
            local v = 1.0

            local desiredEnv = state.env.baseVolume * v
            state.env.currentVolume = desiredEnv
        else
            -- Grace window: the ENV seq id can flicker to 0/nil during transitions.
            -- Don't tear down the streamed ENV immediately, or short jingles can
            -- restart multiple times.
            if state.env.stream ~= nil then
                -- If we're within the one-shot debounce window, keep the stream
                -- alive even if ENV appears to have stopped. Vanilla frequently
                -- re-issues seq_player_play_sequence() for dialog music.
                if state.env.oneShotDebounceFrames > 0 then
                    state.env.noSeqGraceFrames = 0
                    return
                end

                state.env.noSeqGraceFrames = state.env.noSeqGraceFrames + 1
                if state.env.noSeqGraceFrames >= 10 then
                    audio_stream_stop(state.env.stream)
                    audio_stream_destroy(state.env.stream)
                    state.env.stream = nil
                    state.env.path = nil
                    state.env.seqId = nil
                    state.env.baseVolume = 1.0
                    state.env.currentVolume = 0.0
                    state.env.noSeqGraceFrames = 0
                end
            else
                state.env.noSeqGraceFrames = 0
            end
        end
    end

    if not state.boss.defeatedLatch and type(obj_get_first_with_behavior_id) == 'function' then
        local bowser = obj_get_first_with_behavior_id(id_bhvBowser)
        if bowser ~= nil and bowser.oAction == 4 then
            local sub = bowser.oSubAction or -1
            local phase = bowser.oBowserUnkF8 or 0
            -- Trigger at the twirl/fade phase, which begins after the defeated dialog.
            -- Not-BITS: subAction=3, phase>=2. BITS: subAction=10, phase>=2.
            if (sub == 3 and phase >= 2) or (sub == 10 and phase >= 2) or sub >= 4 or sub >= 11 then
                state.boss.defeatedLatch = true
                -- Don't stop the stream immediately here; Bowser 3 can still be
                -- in a dialog/opening state while vanilla boss music continues.
                -- The latch is used below to prevent boss music from restarting.
            end
        end
    end

    local fileSelectActive = false
    if type(is_file_select_active) == 'function' then
        fileSelectActive = is_file_select_active()
    end

    -- Default: don't force any seq
    state.seq.forcedSeqId = nil
    state.flags.muteEnvPlayer = false

    if state.boss.introController ~= nil and type(state.boss.introController.update) == 'function' then
        local function set_forced_seq_id(seqId)
            state.seq.forcedSeqId = seqId
        end
        state.boss.introController.update(set_forced_seq_id, refresh_track)
    end

    -- File select: use the game's real file select state instead of trying to
    -- infer it from seqArgs.
    if fileSelectActive then
        reportedSeqId = SEQ_MENU_FILE_SELECT
        state.seq.forcedSeqId = SEQ_MENU_FILE_SELECT
    end

    -- Vanilla secondary music (Merry-Go-Round, Piranha Plant) and some cutscene
    -- music (e.g. Bowser key) plays on SEQ_PLAYER_ENV.
    -- If ENV is active and we have a streamed mapping for it, play it and pause
    -- the vanilla ENV sequence player to avoid overlap.
    -- ENV is handled by a separate stream; background selection continues to
    -- follow the background music queue.

    -- Once Bowser begins his death sequence, never allow boss music to restart
    -- until leaving/re-entering the level.
    if state.boss.defeatedLatch then
        if reportedSeqId == SEQ_LEVEL_BOSS_KOOPA or reportedSeqId == SEQ_LEVEL_BOSS_KOOPA_FINAL or reportedSeqId == SEQ_EVENT_BOSS then
            -- Don't stop here; vanilla can keep boss music running during dialog
            -- and the early death sequence. We only want to prevent boss music
            -- from restarting once the latch is set.
            update_stream_volume()
            return
        end
    end

    -- Leaving file select: stop the file-select stream immediately so it doesn't
    -- carry into other screens (castle grounds, title, etc.).
    if (not fileSelectActive) and state.seq.lastSeqId == SEQ_MENU_FILE_SELECT then
        if type(stop_background_music) == 'function' then
            stop_background_music(SEQ_MENU_FILE_SELECT)
        end
        stop_stream()
        state.seq.lastSeqId = nil
        state.seq.lastBgSeqId = nil
        state.seq.lastMissingSeqId = nil
        return
    end

    -- Bowser death/key spawn: vanilla queues COLLECT_KEY here; stop the boss stream
    -- immediately so it doesn't overlap the cutscene.
    if reportedSeqId == SEQ_EVENT_CUTSCENE_COLLECT_KEY then
        if state.seq.lastBgSeqId == SEQ_LEVEL_BOSS_KOOPA or state.seq.lastBgSeqId == SEQ_LEVEL_BOSS_KOOPA_FINAL or state.seq.lastBgSeqId == SEQ_EVENT_BOSS then
            stop_stream()
            state.seq.lastSeqId = nil
            state.seq.lastMissingSeqId = nil
        end
    end

    -- Boss death cutscene: when VICTORY starts after boss music, stop boss music immediately
    -- and don't allow any other background track to resume until STAR_SPAWN finishes.
    if reportedSeqId == SEQ_EVENT_CUTSCENE_VICTORY then
        if (state.seq.lastBgSeqId == SEQ_LEVEL_BOSS_KOOPA or state.seq.lastBgSeqId == SEQ_LEVEL_BOSS_KOOPA_FINAL or state.seq.lastBgSeqId == SEQ_EVENT_BOSS) and (not state.boss.cutsceneLock) then
            state.boss.cutsceneLock = true
            state.boss.cutsceneSawStarSpawn = false
            stop_stream()
            state.seq.lastSeqId = nil
            state.seq.lastBgSeqId = nil
            state.seq.lastMissingSeqId = nil
        end
    end

    if state.boss.cutsceneLock then
        -- Track progression through the boss cutscene music chain.
        if reportedSeqId == SEQ_EVENT_CUTSCENE_STAR_SPAWN then
            state.boss.cutsceneSawStarSpawn = true
        end

        -- Allow only the boss cutscene-related sequences to control streamed output.
        if reportedSeqId == SEQ_EVENT_CUTSCENE_VICTORY
            or reportedSeqId == SEQ_EVENT_CUTSCENE_STAR_SPAWN
            or reportedSeqId == SEQ_EVENT_CUTSCENE_COLLECT_STAR then
            state.seq.forcedSeqId = reportedSeqId
        else
            -- If we haven't reached STAR_SPAWN yet, keep holding on whatever we last played.
            if not state.boss.cutsceneSawStarSpawn then
                update_stream_volume()
                return
            end

            -- Once STAR_SPAWN has played, we can release the lock when we leave the cutscene chain.
            state.boss.cutsceneLock = false
            state.boss.cutsceneSawStarSpawn = false
            state.seq.forcedSeqId = nil
        end
    end

    if reportedSeqId == SEQ_EVENT_CUTSCENE_VICTORY
        or reportedSeqId == SEQ_EVENT_CUTSCENE_ENDING
        or reportedSeqId == SEQ_EVENT_CUTSCENE_COLLECT_KEY then
        if state.seq.lastBgSeqId == SEQ_LEVEL_BOSS_KOOPA or state.seq.lastBgSeqId == SEQ_LEVEL_BOSS_KOOPA_FINAL or state.seq.lastBgSeqId == SEQ_EVENT_BOSS then
            state.flags.forceFadeOut = true
        end
    else
        state.flags.forceFadeOut = false
    end

    if reportedSeqId ~= nil and (not is_non_restart_overlay_seq(reportedSeqId)) then
        state.seq.lastBgSeqId = reportedSeqId
    end

    local effectiveSeqId = reportedSeqId
    if is_non_restart_overlay_seq(reportedSeqId) and state.seq.lastBgSeqId ~= nil then
        effectiveSeqId = state.seq.lastBgSeqId
    end

    if effectiveSeqId ~= state.seq.lastSeqId then
        state.seq.lastSeqId = effectiveSeqId
        state.seq.lastMissingSeqId = nil
        state.seq.lastDynamicIndex = dynIndex
        refresh_track()
    elseif dynIndex ~= state.seq.lastDynamicIndex then
        local baseTrack = get_track_for_seq_id(effectiveSeqId)
        if baseTrack ~= nil then
            local phaseTrack = get_phase_track(baseTrack, dynIndex)
            if phaseTrack ~= nil then
                state.seq.lastDynamicIndex = dynIndex
                refresh_track()
            end
        end
    elseif state.bg.stream == nil then
        refresh_track()
    end

    update_stream_volume()
end

local function on_exit()
    unmute_all_sequences()
    stop_stream()
end

hook_event(HOOK_ON_LEVEL_INIT, on_level_init)
hook_event(HOOK_ON_WARP, on_warp)
hook_event(HOOK_ON_INSTANT_WARP, on_instant_warp)
hook_event(HOOK_ON_CLEAR_AREAS, on_clear_areas)
hook_event(HOOK_UPDATE, on_update)
hook_event(HOOK_ON_EXIT, on_exit)

return {}
