-- Maps vanilla seq IDs to Render96ex streamed wav files.
--
-- This intentionally follows the engine's own music selection, so each level/area/cutscene
-- automatically uses the correct track.
--
-- Values:
-- - file: absolute path to wav
-- - loopStart/loopEnd: PCM frame indices for seamless looping
-- - volume: 0.0 - 1.0 base volume (before vanilla fade scaling)

local function vol127(v)
    return (v or 127) / 127.0
end

local T = {
    -- Level / area music
    [3]  = { file = "audio/levels/level_grass.wav", loopStart = 187327, loopEnd = -1, volume = vol127(89) },
    [8]  = { file = "audio/levels/level_snow.wav", loopStart = 138353, loopEnd = -1, volume = vol127(95) },
    [9]  = { file = "audio/levels/level_slide.wav", loopStart = 126754, loopEnd = -1, volume = vol127(95) },
    [5]  = { file = "audio/levels/level_water_phase1.wav", loopStart = 159158, loopEnd = -1, volume = vol127(95), phases = {
        { file = "audio/levels/level_water_phase1.wav", loopStart = 159158, loopEnd = -1, volume = vol127(95) },
        { file = "audio/levels/level_water_phase2.wav", loopStart = 159158, loopEnd = -1, volume = vol127(95) },
        { file = "audio/levels/level_water_phase3.wav", loopStart = 159158, loopEnd = -1, volume = vol127(95) },
    } },
    [6]  = { file = "audio/levels/level_hot.wav", loopStart = 716332, loopEnd = -1, volume = vol127(92) },
    [10] = { file = "audio/levels/level_spooky_phase1.wav", loopStart = 905211, loopEnd = -1, volume = vol127(102), phases = {
        { file = "audio/levels/level_spooky_phase1.wav", loopStart = 905211, loopEnd = -1, volume = vol127(102) },
        { file = "audio/levels/level_spooky_phase2.wav", loopStart = 905211, loopEnd = -1, volume = vol127(102) },
    } },
    [12] = { file = "audio/levels/level_underground_phase1.wav", loopStart = 1597796, loopEnd = -1, volume = vol127(127), phases = {
        { file = "audio/levels/level_underground_phase1.wav", loopStart = 1597796, loopEnd = -1, volume = vol127(127) },
        { file = "audio/levels/level_underground_phase2.wav", loopStart = 1597796, loopEnd = -1, volume = vol127(127) },
    } },
    [17] = { file = "audio/levels/level_koopa_road.wav", loopStart = 156800, loopEnd = -1, volume = vol127(92) },
    [7]  = { file = "audio/levels/level_boss_koopa.wav", loopStart = 222451, loopEnd = -1, volume = vol127(108) },
    [25] = { file = "audio/levels/level_boss_koopa_final.wav", loopStart = 1117670, loopEnd = -1, volume = vol127(108) },
    [4]  = { file = "audio/levels/level_inside_castle.wav", loopStart = 22810, loopEnd = -1, volume = vol127(110) },

    -- These are custom Render96ex sequences (not vanilla sm64); keep them by numeric IDs if present in this build.
    -- [??] = { file = "audio/levels/level_fourth_floor.wav", loopStart = 1, loopEnd = -1, volume = vol127(80) },
    [30] = { file = "audio/levels/level_castle_courtyard.wav", loopStart = 1, loopEnd = -1, volume = vol127(80) },
    [31] = { file = "audio/levels/level_castle_grounds.wav", loopStart = 1, loopEnd = -1, volume = vol127(80) },

    -- Event / special sequences
    [22] = { file = "audio/jingles/event_boss_theme.wav", loopStart = 124950, loopEnd = -1, volume = vol127(114) },
    [35] = { file = "audio/jingles/event_boss_intro.wav", loopStart = 1, loopEnd = -1, loop = true, volume = vol127(114) },
    [15] = { file = "audio/jingles/event_cap_metal.wav", loopStart = 180000, loopEnd = -1, volume = vol127(102) },
    [20] = { file = "audio/jingles/event_race_fanfare.wav", loopStart = 1, loopEnd = -1, volume = vol127(102) },
    [14] = { file = "audio/jingles/event_powerup.wav", loopStart = 372400, loopEnd = -1, volume = vol127(82) },

    -- Menus / jingles (non-looping unless loop=true)
    [2]  = { file = "audio/jingles/event_title_screen.wav", loopStart = 123070, loopEnd = -1, volume = vol127(89) },
    [1]  = { file = "audio/jingles/event_star_collect.wav", loopStart = 1, loopEnd = -1, volume = vol127(72) },
    [13] = { file = "audio/jingles/event_star_select.wav", loopStart = 1, loopEnd = -1, volume = vol127(82) },
    [18] = { file = "audio/jingles/event_star_fanfare.wav", loopStart = 1, loopEnd = -1, volume = vol127(89) },
    [21] = { file = "audio/jingles/event_star_appear.wav", loopStart = 1, loopEnd = -1, volume = vol127(89) },
    [33] = { file = "audio/jingles/menu_file_select.wav", loopStart = 1, loopEnd = -1, loop = true, volume = vol127(82) },
    [34] = { file = "audio/jingles/event_lakitu_message.wav", loopStart = 1, loopEnd = -1, volume = vol127(75) },
    [23] = { file = "audio/jingles/event_key_collect.wav", loopStart = 1, loopEnd = -1, volume = vol127(82) },
    [37] = { file = "audio/jingles/event_game_over.wav", loopStart = 556587, loopEnd = -1, volume = vol127(89) },
    [24] = { file = "audio/jingles/event_endless_stairs.wav", loopStart = 1, loopEnd = -1, volume = vol127(89) },
    [19] = { file = "audio/jingles/event_merry_go_round.wav", loopStart = 1, loopEnd = -1, loop = true, volume = vol127(95) },
    [11] = { file = "audio/jingles/event_piranha_plant.wav", loopStart = 1, loopEnd = -1, volume = vol127(89) },
    [27] = { file = "audio/jingles/event_solve_puzzle.wav", loopStart = 1, loopEnd = -1, volume = vol127(82) },
    [28] = { file = "audio/jingles/event_toad_message.wav", loopStart = 1, loopEnd = -1, volume = vol127(102) },
    [29] = { file = "audio/jingles/event_peach_message.wav", loopStart = 1, loopEnd = -1, volume = vol127(89) },
    [30] = { file = "audio/jingles/event_intro.wav", loopStart = 1, loopEnd = -1, volume = vol127(95) },
    [31] = { file = "audio/jingles/event_victory.wav", loopStart = 1, loopEnd = -1, volume = vol127(95) },
    [32] = { file = "audio/jingles/event_peach_ending.wav", loopStart = 1, loopEnd = -1, volume = vol127(89) },
    [26] = { file = "audio/jingles/event_credits.wav", loopStart = 1, loopEnd = -1, volume = vol127(102) },
    [16] = { file = "audio/jingles/event_koopa_message.wav", loopStart = 1, loopEnd = -1, volume = vol127(82) },
}

return T
