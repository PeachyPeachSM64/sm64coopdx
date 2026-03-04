local config = require('config')

--- @type table<Object, { x: number, y: number, z: number }>
local sLastPosBy1Up = {}

--- @type table<Object, integer>
local sLightIdByObj = setmetatable({}, { __mode = 'k' })

--- @param o Object
local function is_1up(o)
    if o == nil then return false end
    if type(obj_is_mushroom_1up) == 'function' then
        return obj_is_mushroom_1up(o)
    end

    return obj_has_behavior_id(o, id_bhv1Up) == 1
        or obj_has_behavior_id(o, id_bhv1upJumpOnApproach) == 1
        or obj_has_behavior_id(o, id_bhv1upRunningAway) == 1
        or obj_has_behavior_id(o, id_bhv1upSliding) == 1
        or obj_has_behavior_id(o, id_bhv1upWalking) == 1
        or obj_has_behavior_id(o, id_bhvHidden1up) == 1
        or obj_has_behavior_id(o, id_bhvHidden1upInPole) == 1
        or obj_has_behavior_id(o, id_bhvHidden1upInPoleSpawner) == 1
        or obj_has_behavior_id(o, id_bhvHidden1upInPoleTrigger) == 1
        or obj_has_behavior_id(o, id_bhvHidden1upTrigger) == 1
end

--- @param o Object
local function remove_1up_light(o)
    if o == nil then return end

    local id = sLightIdByObj[o]
    if id ~= nil and id ~= 0 and le_light_exists(id) then
        le_remove_light(id)
    end

    sLightIdByObj[o] = nil
    sLastPosBy1Up[o] = nil
end

--- @param o Object
local function ensure_1up_light(o)
    if o == nil then return end

    local existing = sLightIdByObj[o]
    if existing ~= nil and existing ~= 0 and le_light_exists(existing) then
        return
    end

    local y = o.oPosY
    if o.oGraphYOffset ~= nil then
        y = y + o.oGraphYOffset
    end

    local id = le_add_light(
        o.oPosX, y, o.oPosZ,
        config.ONEUP_LIGHT_COLOR_R, config.ONEUP_LIGHT_COLOR_G, config.ONEUP_LIGHT_COLOR_B,
        config.ONEUP_LIGHT_RADIUS, config.ONEUP_LIGHT_INTENSITY
    )

    sLightIdByObj[o] = id
    sLastPosBy1Up[o] = { x = o.oPosX, y = y, z = o.oPosZ }
end

--- @param o Object
local function update_1up_light(o)
    ensure_1up_light(o)

    local id = sLightIdByObj[o]
    if id ~= nil and id ~= 0 and le_light_exists(id) then
        local y = o.oPosY
        if o.oGraphYOffset ~= nil then
            y = y + o.oGraphYOffset
        end

        local last = sLastPosBy1Up[o]
        if last == nil
            or math.abs(last.x - o.oPosX) > config.POS_EPSILON
            or math.abs(last.y - y) > config.POS_EPSILON
            or math.abs(last.z - o.oPosZ) > config.POS_EPSILON then
            le_set_light_pos(id, o.oPosX, y, o.oPosZ)
            sLastPosBy1Up[o] = { x = o.oPosX, y = y, z = o.oPosZ }
        end
    end
end

--- @param unloadedObject Object
local function on_object_unload_1up(unloadedObject)
    if not is_1up(unloadedObject) then return end
    remove_1up_light(unloadedObject)
end

--- @param o Object
local function bhv_1up_light_init(o)
    if not is_1up(o) then return end
    ensure_1up_light(o)
end

--- @param o Object
local function bhv_1up_light_loop(o)
    if not is_1up(o) then return end
    update_1up_light(o)
end

hook_behavior(id_bhv1Up, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhv1upJumpOnApproach, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhv1upRunningAway, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhv1upSliding, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhv1upWalking, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhvHidden1up, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhvHidden1upInPole, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhvHidden1upInPoleSpawner, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhvHidden1upInPoleTrigger, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)
hook_behavior(id_bhvHidden1upTrigger, OBJ_LIST_LEVEL, false, bhv_1up_light_init, bhv_1up_light_loop)

hook_event(HOOK_ON_OBJECT_UNLOAD, on_object_unload_1up)
