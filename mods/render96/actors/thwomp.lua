-- thwomp.lua

local SHAKE_TICKS = 18
local SHAKE_POS_MAG = 10.0
local SHAKE_ANGLE_MAG = 0x120

local function render96_thwomp_prefall_shake(o)
    if o == nil then return end

    -- Thwomp action state machine:
    -- 0 = rising, 1 = waiting (pre-fall), 2 = falling, 3 = landed, 4 = cooldown
    if o.oAction ~= 1 then
        return
    end

    if o.oThwompRandomTimer == nil or o.oTimer == nil then
        return
    end

    local remaining = o.oThwompRandomTimer - o.oTimer
    if remaining > (SHAKE_TICKS + 0.5) or remaining < 0 then
        return
    end

    local t = o.oTimer

    -- Visual-only shake (does not affect collision)
    local ox = (math.sin(t * 6.9) + math.sin(t * 15.3)) * 0.5 * SHAKE_POS_MAG
    local oz = (math.cos(t * 8.1) + math.cos(t * 14.1)) * 0.5 * SHAKE_POS_MAG

    o.oPosX = o.oHomeX + ox
    o.oPosZ = o.oHomeZ + oz

    local yawJitter = math.floor(math.sin(t * 18.0) * SHAKE_ANGLE_MAG)
    local rollJitter = math.floor(math.cos(t * 21.0) * (SHAKE_ANGLE_MAG / 2))

    o.oFaceAngleYaw = o.oMoveAngleYaw + yawJitter
    o.oFaceAngleRoll = rollJitter
end

hook_behavior(id_bhvThwomp, OBJ_LIST_SURFACE, false, nil, render96_thwomp_prefall_shake)
hook_behavior(id_bhvThwomp2, OBJ_LIST_SURFACE, false, nil, render96_thwomp_prefall_shake)
