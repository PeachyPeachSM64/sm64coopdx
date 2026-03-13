-- name: Enhanced File Select
-- description: Slightly scales file select buttons on hover and triggers a slight rumble.
-- author: \#40e740\ExcellentGamer

local HOVER_SCALE_MULT = 1.1
local HOVER_RUMBLE_LENGTH = 2
local HOVER_RUMBLE_STRENGTH = 20

local sBaseScale = {}
local sHoverApplied = {}
local sPrevHoveredKey = nil

local function get_obj_key(o)
    return tostring(o)
end

local function get_or_init_base_scale(o)
    local key = get_obj_key(o)
    local base = sBaseScale[key]
    if base == nil then
        base = o.oMenuButtonScale
        sBaseScale[key] = base
    end
    return base
end

local function is_cursor_over_button(cursorX, cursorY, buttonX, buttonY, depth)
    local a = 52.4213
    local newX = (buttonX * 160.0) / (a * depth)
    local newY = (buttonY * 120.0) / (a * 3.0 / 4.0 * depth)

    local maxX = newX + 25.0
    local minX = newX - 25.0
    local maxY = newY + 21.0
    local minY = newY - 21.0

    return (cursorX < maxX and minX < cursorX and cursorY < maxY and minY < cursorY)
end

local function enhanced_file_select_update()
    if not is_file_select_active() then
        sPrevHoveredKey = nil
        return
    end

    local cursorX, cursorY = file_select_get_cursor_pos()

    local hovered = nil
    local o = obj_get_first_with_behavior_id(id_bhvMenuButton)
    while o ~= nil do
        if o.oMenuButtonState == 0 then
            local base = get_or_init_base_scale(o)
            local depth = (base < 0.5) and 22.0 or 200.0

            if is_cursor_over_button(cursorX, cursorY, o.oPosX, o.oPosY, depth) then
                hovered = o
            end
        end

        o = obj_get_next_with_same_behavior_id(o)
    end

    local hoveredKey = hovered and get_obj_key(hovered) or nil

    o = obj_get_first_with_behavior_id(id_bhvMenuButton)
    while o ~= nil do
        local key = get_obj_key(o)
        local base = get_or_init_base_scale(o)

        if o.oMenuButtonState == 0 then
            if hoveredKey ~= nil and key == hoveredKey then
                o.oMenuButtonScale = base * HOVER_SCALE_MULT
                sHoverApplied[key] = true
            else
                if sHoverApplied[key] then
                    o.oMenuButtonScale = base
                    sHoverApplied[key] = false
                end
            end
        else
            if sHoverApplied[key] then
                o.oMenuButtonScale = base
                sHoverApplied[key] = false
            end
        end

        o = obj_get_next_with_same_behavior_id(o)
    end

    if hoveredKey ~= nil and hoveredKey ~= sPrevHoveredKey then
        local m = gMarioStates[0]
        if m ~= nil then
            queue_rumble_data_mario(m, HOVER_RUMBLE_LENGTH, HOVER_RUMBLE_STRENGTH)
        else
            queue_rumble_data(HOVER_RUMBLE_LENGTH, HOVER_RUMBLE_STRENGTH)
        end
    end

    sPrevHoveredKey = hoveredKey
end

hook_event(HOOK_UPDATE, enhanced_file_select_update)
