local config = require('config')

local M = {}

local sCurrentLevelNum = nil

M.debugYawDeg = nil
M.debugPitchDeg = nil
M.debugIntensity = nil
M.debugModeActive = false

local function clamp(v, lo, hi)
    if v < lo then return lo end
    if v > hi then return hi end
    return v
end

local function deg_to_unit(deg)
    return deg * 182.04444444444445
end

local function apply_dir_light_if_needed()
    local levelNum = sCurrentLevelNum
    if type(levelNum) ~= 'number' then return end

    local ovr = config.DIR_LIGHT_BY_LEVEL[levelNum]

    local yawDeg = config.DEFAULT_LEVEL_DIR_LIGHT_YAW_DEG
    local pitchDeg = config.DEFAULT_LEVEL_DIR_LIGHT_PITCH_DEG
    local intensity = config.DEFAULT_LEVEL_DIR_LIGHT_INTENSITY

    if ovr ~= nil then
        if type(ovr.yawDeg) == 'number' then yawDeg = ovr.yawDeg end
        if type(ovr.pitchDeg) == 'number' then pitchDeg = ovr.pitchDeg end
        if type(ovr.intensity) == 'number' then intensity = ovr.intensity end
    end

    if M.debugModeActive then
        if M.debugYawDeg ~= nil then yawDeg = M.debugYawDeg end
        if M.debugPitchDeg ~= nil then pitchDeg = M.debugPitchDeg end
        if M.debugIntensity ~= nil then intensity = M.debugIntensity end
    end

    intensity = clamp(intensity, 0.0, 1.0)

    local yaw = deg_to_unit(yawDeg)
    local pitch = deg_to_unit(pitchDeg)

    local cp = coss(pitch)
    local sp = sins(pitch)
    local sy = sins(yaw)
    local cy = coss(yaw)

    -- World-space direction based on configured yaw/pitch.
    local wx = cp * sy
    local wy = sp
    local wz = cp * cy

    set_lighting_dir(0, wx * intensity)
    set_lighting_dir(1, wy * intensity)
    set_lighting_dir(2, wz * intensity)
end

local function on_level_init(_, levelNum, _, _, _)
    sCurrentLevelNum = levelNum
    apply_dir_light_if_needed()
end

local function on_update()
    -- Re-apply each frame to override any camera-based / other mod changes.
    apply_dir_light_if_needed()
end

hook_event(HOOK_ON_LEVEL_INIT, on_level_init)
hook_event(HOOK_UPDATE, on_update)

M.getCurrentLevelDefaults = function()
    local levelNum = sCurrentLevelNum
    local ovr = config.DIR_LIGHT_BY_LEVEL[levelNum]
    local yawDeg = config.DEFAULT_LEVEL_DIR_LIGHT_YAW_DEG
    local pitchDeg = config.DEFAULT_LEVEL_DIR_LIGHT_PITCH_DEG
    local intensity = config.DEFAULT_LEVEL_DIR_LIGHT_INTENSITY
    if ovr ~= nil then
        if type(ovr.yawDeg) == 'number' then yawDeg = ovr.yawDeg end
        if type(ovr.pitchDeg) == 'number' then pitchDeg = ovr.pitchDeg end
        if type(ovr.intensity) == 'number' then intensity = ovr.intensity end
    end
    return yawDeg, pitchDeg, intensity
end

return M
