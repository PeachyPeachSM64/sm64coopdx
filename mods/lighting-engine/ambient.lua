local config = require('config')

local M = {}

local sCurrentLevelNum = nil
local sCurrentAreaIndex = nil
local sWaterFadeFactor = 0.0
local sTargetAmbientR = 255
local sTargetAmbientG = 255
local sTargetAmbientB = 255

local function is_camera_underwater()
    local camPos = gLakituState.pos
    if camPos == nil then return false end

    local waterLevel = find_water_level(camPos.x, camPos.z)
    return camPos.y < waterLevel
end

local function apply_ambient_with_water_fade()
    local r = sTargetAmbientR
    local g = sTargetAmbientG
    local b = sTargetAmbientB

    if sWaterFadeFactor > 0 then
        local waterR = r * config.WATER_AMBIENT_MULT_R
        local waterG = g * config.WATER_AMBIENT_MULT_G
        local waterB = b * config.WATER_AMBIENT_MULT_B

        r = r + (waterR - r) * sWaterFadeFactor
        g = g + (waterG - g) * sWaterFadeFactor
        b = b + (waterB - b) * sWaterFadeFactor
    end

    le_set_ambient_color(math.floor(r), math.floor(g), math.floor(b))
end

local function get_level_area_ambient(levelNum, areaIndex)
    local levelAreas = config.AMBIENT_BY_LEVEL_AREA[levelNum]
    if levelAreas ~= nil then
        local ovr = levelAreas[areaIndex]
        if ovr ~= nil then
            return ovr.r, ovr.g, ovr.b
        end
    end

    local levelOvr = config.AMBIENT_BY_LEVEL[levelNum]
    if levelOvr ~= nil then
        return levelOvr.r, levelOvr.g, levelOvr.b
    end

    return config.DEFAULT_LEVEL_AMBIENT_R, config.DEFAULT_LEVEL_AMBIENT_G, config.DEFAULT_LEVEL_AMBIENT_B
end

local function update_ambient_if_needed()
    local m = gMarioStates[0]
    if m == nil or m.area == nil then return end

    local areaIndex = m.area.index
    if type(areaIndex) ~= 'number' then return end

    local levelNum = sCurrentLevelNum
    if type(levelNum) ~= 'number' then return end

    if sCurrentAreaIndex == areaIndex then return end
    sCurrentAreaIndex = areaIndex

    local r, g, b = get_level_area_ambient(levelNum, areaIndex)
    sTargetAmbientR = r
    sTargetAmbientG = g
    sTargetAmbientB = b
    apply_ambient_with_water_fade()
end

function M.get_current_level_num()
    return sCurrentLevelNum
end

local function update_water_fade()
    local underwater = is_camera_underwater()
    local targetFade = underwater and 1.0 or 0.0
    local fadeSpeed = config.WATER_AMBIENT_FADE_SPEED

    if sWaterFadeFactor < targetFade then
        sWaterFadeFactor = math.min(sWaterFadeFactor + fadeSpeed, targetFade)
    elseif sWaterFadeFactor > targetFade then
        sWaterFadeFactor = math.max(sWaterFadeFactor - fadeSpeed, targetFade)
    end
end

local function on_level_init(_, levelNum, _, _, _)
    le_set_mode(LE_MODE_AFFECT_ALL_SHADED_AND_COLORED)
    sCurrentLevelNum = levelNum
    sCurrentAreaIndex = nil
    sWaterFadeFactor = 0.0

    update_ambient_if_needed()
end

local function on_update()
    update_ambient_if_needed()
    update_water_fade()
    apply_ambient_with_water_fade()
end

hook_event(HOOK_ON_LEVEL_INIT, on_level_init)
hook_event(HOOK_UPDATE, on_update)

return M
