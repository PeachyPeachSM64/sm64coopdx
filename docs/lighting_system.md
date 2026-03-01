# Lighting System Documentation

## Overview

The Render96DX lighting system provides optional per-pixel lighting with support for:
- Point lights (omnidirectional with radius-based attenuation)
- Directional lights (infinite distance, e.g., sun/moon)
- Spot lights (cone-shaped with falloff)
- Shadow mapping (optional, per-light)
- Customizable shader packs (Minecraft Java Edition style)

The system is completely optional and can be toggled on/off at runtime.

## Configuration

Add these options to your `sm64config.txt`:

```
lighting_enabled 0                     # Enable/disable lighting system (0=off, 1=on)
lighting_shadows_enabled 0             # Enable shadow mapping (0=off, 1=on)
lighting_shadow_quality 2              # Shadow quality (0=low, 1=medium, 2=high, 3=ultra)
lighting_shader_packs_enabled 0        # Enable custom shader packs (0=off, 1=on)
lighting_active_shader_pack "default"  # Active shader pack name
```

## Lua API

### Initialization

```lua
lighting_init()           -- Initialize lighting system
lighting_shutdown()       -- Shutdown lighting system
lighting_reset()          -- Remove all lights
```

### Adding Lights

```lua
-- Add point light
-- Returns: light index (integer)
local lightId = lighting_add_point_light(
    x, y, z,              -- Position
    {r=1, g=1, b=1},      -- Color (0-1 range)
    1.0,                  -- Intensity
    1000.0                -- Radius
)

-- Add directional light (e.g., sun)
local sunId = lighting_add_directional_light(
    dirX, dirY, dirZ,     -- Direction (normalized)
    {r=1, g=0.95, b=0.8}, -- Color
    1.0                   -- Intensity
)

-- Add spot light
local spotId = lighting_add_spot_light(
    x, y, z,              -- Position
    dirX, dirY, dirZ,     -- Direction
    {r=1, g=1, b=1},      -- Color
    1.0,                  -- Intensity
    500.0,                -- Radius
    12.5,                 -- Inner cone angle (degrees)
    17.5                  -- Outer cone angle (degrees)
)
```

### Managing Lights

```lua
lighting_remove_light(lightId)              -- Remove a light
lighting_enable_light(lightId, true)        -- Enable/disable a light
lighting_set_light_shadow(lightId, true)    -- Enable/disable shadows for a light
```

### Global Settings

```lua
-- Set ambient lighting
lighting_set_ambient({r=0.3, g=0.3, b=0.3}, 1.0)

-- Enable/disable entire system
lighting_enable(true)
lighting_enable_shadows(true)

-- Query status
local enabled = lighting_get_enabled()
local shadowsEnabled = lighting_get_shadows_enabled()
```

### Shader Packs

```lua
shader_pack_set_enabled(true)       -- Enable custom shaders
local count = shader_pack_get_count()  -- Get number of loaded packs
shader_pack_activate(0)             -- Activate pack by index
shader_pack_deactivate()            -- Deactivate custom shaders
local enabled = shader_pack_is_enabled()
```

## Example Usage

### Simple Point Light

```lua
-- Add a torch-like light
function create_torch_light(x, y, z)
    return lighting_add_point_light(
        x, y, z,
        {r=1.0, g=0.7, b=0.3},  -- Warm orange color
        2.0,                     -- Bright
        800.0                    -- Medium radius
    )
end
```

### Dynamic Sunlight

```lua
local sunLightId = nil

function update_sunlight()
    -- Remove old sun if exists
    if sunLightId then
        lighting_remove_light(sunLightId)
    end
    
    -- Calculate sun direction based on time
    local time = get_time_of_day()
    local angle = (time / 24.0) * 2 * math.pi
    
    local dirX = math.sin(angle)
    local dirY = -math.cos(angle)
    local dirZ = 0
    
    -- Sun color changes with time
    local color = {r=1.0, g=0.95, b=0.8}
    if time < 6 or time > 18 then
        color = {r=0.1, g=0.1, b=0.3}  -- Night (moonlight)
    end
    
    sunLightId = lighting_add_directional_light(
        dirX, dirY, dirZ,
        color,
        1.0
    )
    
    lighting_set_light_shadow(sunLightId, true)
end
```

### Spotlight Example

```lua
-- Create a flashlight effect
function create_flashlight(object)
    local pos = {
        object.oPosX,
        object.oPosY + 100,
        object.oPosZ
    }
    
    local dir = {
        math.sin(object.oFaceAngleYaw),
        -0.2,
        math.cos(object.oFaceAngleYaw)
    }
    
    return lighting_add_spot_light(
        pos[1], pos[2], pos[3],
        dir[1], dir[2], dir[3],
        {r=1.0, g=1.0, b=0.9},
        2.0,   -- Intensity
        1500.0, -- Radius
        15.0,  -- Inner angle
        25.0   -- Outer angle
    )
end
```

## Creating Custom Shader Packs

Shader packs are stored in the `shaderpacks/` directory. Each pack is a folder containing:

```
shaderpacks/
  mypack/
    shaderpack.properties   # Pack metadata
    shaders/
      composite.vsh         # Composite vertex shader
      composite.fsh         # Composite fragment shader
      final.vsh             # Final pass vertex shader
      final.fsh             # Final pass fragment shader
      shadow.vsh            # Shadow vertex shader (optional)
      shadow.fsh            # Shadow fragment shader (optional)
```

### shaderpack.properties

```ini
author=Your Name
version=1.0.0
description=My custom lighting shader
shadowDistance=10000.0
dynamicHandLight=true
oldLighting=false
```

### Shader Uniforms

Your shaders will receive these uniforms:

```glsl
// Lighting uniforms
uniform int uLightingEnabled;
uniform int uNumLights;
uniform vec3 uAmbientLight;
uniform float uAmbientIntensity;
uniform vec3 uViewPos;

// Light array (MAX_LIGHTS = 32)
struct Light {
    int type;              // 0=none, 1=point, 2=directional, 3=spot
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float radius;
    vec3 attenuation;
    float innerCutoff;
    float outerCutoff;
};
uniform Light uLights[32];

// Textures
uniform sampler2D uTex0;           // Base texture
uniform sampler2D uShadowMap[8];   // Shadow maps (if enabled)

// Matrices
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;
```

### Basic Lighting Calculation

See `shaderpacks/default/shaders/composite.fsh` for a complete example.

## Performance Considerations

- **Light Count**: Up to 32 lights supported, but 8-16 active lights recommended
- **Shadow Maps**: Each shadow-casting light requires a 2048x2048 depth texture
- **Shader Complexity**: Custom shaders run per-pixel; keep calculations efficient
- **Shadow Quality**: Higher quality = larger shadow maps = more VRAM

## Tips

1. Use directional lights for outdoor scenes (sun/moon)
2. Use point lights for torches, lamps, fire
3. Use spot lights sparingly for special effects
4. Enable shadows only on key lights (e.g., sun)
5. Adjust ambient light to prevent completely black areas
6. Test your shader packs on different hardware

## Troubleshooting

**Lighting not visible**
- Check `lighting_enabled` is set to 1
- Verify lights are within range of geometry
- Check ambient light isn't too bright

**Poor performance**
- Reduce number of active lights
- Disable shadows or reduce shadow quality
- Simplify custom shaders

**Shadows not rendering**
- Ensure `lighting_shadows_enabled` is 1
- Check lights have shadows enabled via `lighting_set_light_shadow()`
- Verify OpenGL 3.0+ support

## Technical Details

- Uses deferred-style lighting calculations
- Shadow mapping with PCF filtering
- Point light shadows use cubemaps
- Directional/spot lights use 2D depth maps
- Attenuation uses inverse-square falloff with radius clamping
