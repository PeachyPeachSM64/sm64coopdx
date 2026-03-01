# Shadow Map System

## Overview

The shadow map system provides configurable, per-pixel shadow rendering for the lighting system. It supports multiple shadow quality presets and per-light shadow control.

## Shadow Quality Levels

Shadow quality determines the resolution of shadow maps:

- **0 - OFF**: Shadows disabled (0x0)
- **1 - LOW**: 512x512 shadow maps
- **2 - MEDIUM**: 1024x1024 shadow maps (default)
- **3 - HIGH**: 2048x2048 shadow maps
- **4 - ULTRA**: 4096x4096 shadow maps

## Configuration

### Via Config File (`sm64config.txt`)

```
lighting_shadows_enabled 1
lighting_shadow_quality 2
```

### Via Lua

```lua
-- Set shadow quality (0-4)
lighting_set_shadow_quality(2)  -- Medium

-- Enable/disable shadows
lighting_enable_shadows(true)

-- Query current settings
local quality = lighting_get_shadow_quality()
local mapSize = lighting_get_shadow_map_size()
local enabled = lighting_get_shadows_enabled()
```

## Per-Light Shadow Control

Each light can individually cast shadows:

```lua
-- Create light with shadows
local sunLight = lighting_add_directional_light(0.5, -0.7, 0.3, {r=1.0, g=0.95, b=0.85}, 1.2)
lighting_set_light_shadow(sunLight, true)  -- Enable shadow casting

-- Create light without shadows
local ambientPoint = lighting_add_point_light(0, 100, 0, {r=1.0, g=1.0, b=1.0}, 1.0, 500.0)
lighting_set_light_shadow(ambientPoint, false)  -- Disable shadow casting
```

## Shadow Map Types

- **Directional Lights**: Use 2D shadow maps
- **Point Lights**: Use cubemap shadow maps (6 faces)
- **Spot Lights**: Use 2D shadow maps

## Performance Considerations

- Higher shadow quality significantly increases VRAM usage and rendering cost
- Each shadow-casting light requires its own shadow map
- Point lights are most expensive (require cubemap rendering)
- Limit shadow-casting lights to important light sources (sun, primary area lights)

## Technical Details

- Shadow maps are depth-only textures
- Linear filtering for soft shadow edges
- Configurable bias to prevent shadow acne
- Supports up to 8 shadow-casting lights simultaneously (hardware dependent)

## Example Usage

```lua
function on_level_init()
    lighting_init()
    lighting_enable(true)
    
    -- Configure shadows
    lighting_set_shadow_quality(3)  -- High quality
    lighting_enable_shadows(true)
    
    -- Sun with shadows
    local sun = lighting_add_directional_light(
        0.5, -0.7, 0.3,
        {r=1.0, g=0.95, b=0.85},
        1.2
    )
    lighting_set_light_shadow(sun, true)
    
    print("Shadow system: " .. lighting_get_shadow_map_size() .. "x" .. lighting_get_shadow_map_size())
end
```
