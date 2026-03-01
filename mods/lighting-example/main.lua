-- Example Lighting Mod for Render96DX
-- Demonstrates modern per-pixel lighting with shadow maps

local lights = {}
local sunLight = nil

function on_level_init()
    -- Initialize lighting system
    lighting_init()
    lighting_enable(true)
    
    -- Configure shadow system
    -- Quality levels: 0=OFF, 1=LOW(512), 2=MEDIUM(1024), 3=HIGH(2048), 4=ULTRA(4096)
    lighting_set_shadow_quality(2)  -- Medium quality (1024x1024)
    lighting_enable_shadows(true)
    
    local shadowSize = lighting_get_shadow_map_size()
    print("Shadow maps enabled - Resolution: " .. shadowSize .. "x" .. shadowSize)
    
    -- Set ambient lighting similar to modern games
    lighting_set_ambient({r=0.3, g=0.3, b=0.4}, 0.6)
    
    -- Add sun as main directional light with shadows
    sunLight = lighting_add_directional_light(
        0.5, -0.7, 0.3,  -- Sun direction (from southeast, downward)
        {r=1.0, g=0.95, b=0.85},  -- Warm daylight color
        1.2  -- Intensity
    )
    lighting_set_light_shadow(sunLight, true)  -- Enable shadow casting
    table.insert(lights, sunLight)
    
    -- Add static point light with shadows
    local staticLight = lighting_add_point_light(
        -1330, 495, 4755,  -- Castle Grounds position
        {r=1.0, g=0.7, b=0.4},  -- Warm orange light
        2.5,  -- Intensity
        2000.0  -- Radius
    )
    lighting_set_light_shadow(staticLight, true)  -- Enable shadow casting
    table.insert(lights, staticLight)
    
    print("Modern per-pixel lighting with shadow maps initialized")
    print("Sun light: Directional with shadows")
    print("Static light: Point at Castle Grounds with shadows")
end

function on_player_connected(player)
    -- Add a light that follows the player
    local playerId = player.globalIndex
    
    if not lights[playerId] then
        local m = gMarioStates[playerId]
        
        -- Create point light at player position
        local lightId = lighting_add_point_light(
            m.pos.x, m.pos.y + 150, m.pos.z,
            {r=1.0, g=0.9, b=0.7},
            1.5,
            1200.0
        )
        
        lights[playerId] = lightId
        print("Added player light: " .. lightId)
    end
end

function mario_update(m)
    -- Update player light position
    local playerId = m.playerIndex
    
    if lights[playerId] then
        lighting_remove_light(lights[playerId])
        
        -- Recreate at new position
        local lightId = lighting_add_point_light(
            m.pos.x, m.pos.y + 150, m.pos.z,
            {r=1.0, g=0.9, b=0.7},
            1.5,
            1200.0
        )
        
        lights[playerId] = lightId
    end
end

function on_hud_render()
    local enabled = lighting_get_enabled()
    local shadowsEnabled = lighting_get_shadows_enabled()
    
    djui_hud_set_color(255, 255, 255, 255)
    djui_hud_print_text("Lighting: " .. (enabled and "ON" or "OFF"), 10, 10, 1)
    djui_hud_print_text("Shadows: " .. (shadowsEnabled and "ON" or "OFF"), 10, 30, 1)
    djui_hud_print_text("Active Lights: " .. #lights, 10, 50, 1)
end

hook_event(HOOK_ON_LEVEL_INIT, on_level_init)
hook_event(HOOK_ON_PLAYER_CONNECTED, on_player_connected)
hook_event(HOOK_MARIO_UPDATE, mario_update)
hook_event(HOOK_ON_HUD_RENDER, on_hud_render)
