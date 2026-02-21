local config = require('config')
local ambient = require('ambient')

local sCurrentLevelNum = nil

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

    local lakitu = gLakituState
    if lakitu == nil then return end

    local ovr = config.DIR_LIGHT_BY_LEVEL[levelNum]

    local yawDeg = config.DEFAULT_LEVEL_DIR_LIGHT_YAW_DEG
    local pitchDeg = config.DEFAULT_LEVEL_DIR_LIGHT_PITCH_DEG
    local intensity = config.DEFAULT_LEVEL_DIR_LIGHT_INTENSITY

    if ovr ~= nil then
        if type(ovr.yawDeg) == 'number' then yawDeg = ovr.yawDeg end
        if type(ovr.pitchDeg) == 'number' then pitchDeg = ovr.pitchDeg end
        if type(ovr.intensity) == 'number' then intensity = ovr.intensity end
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

    -- The renderer later multiplies this direction by the current modelview matrix
    -- (see gfx_pc.c calculate_normal_dir). To keep lighting fixed in world-space,
    -- we must pre-rotate the configured world direction by the camera rotation.
    local camYaw = lakitu.yaw
    local camPitch = lakitu.oldPitch
    local camRoll = lakitu.roll
    if type(camYaw) ~= 'number' then camYaw = 0 end
    if type(camPitch) ~= 'number' then camPitch = 0 end
    if type(camRoll) ~= 'number' then camRoll = 0 end

    -- Camera yaw (rotate around Y)
    local cy2 = coss(camYaw)
    local sy2 = sins(camYaw)
    local x1 = (wx * cy2) - (wz * sy2)
    local y1 = wy
    local z1 = (wx * sy2) + (wz * cy2)

    -- Camera pitch (rotate around X)
    local cp2 = coss(camPitch)
    local sp2 = sins(camPitch)
    local x2 = x1
    local y2 = (y1 * cp2) + (z1 * sp2)
    local z2 = (-y1 * sp2) + (z1 * cp2)

    -- Camera roll (rotate around Z)
    local cr2 = coss(camRoll)
    local sr2 = sins(camRoll)
    local x3 = (x2 * cr2) + (y2 * sr2)
    local y3 = (-x2 * sr2) + (y2 * cr2)
    local z3 = z2

    set_lighting_dir(0, x3 * intensity)
    set_lighting_dir(1, y3 * intensity)
    set_lighting_dir(2, z3 * intensity)
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

return {}
