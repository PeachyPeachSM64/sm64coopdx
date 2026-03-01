# Lighting System Integration Guide

## Overview

This document describes how the lighting system integrates with Render96DX and what changes were made.

## Architecture

The lighting system is designed as an optional, modular extension to the existing renderer:

### Core Components

1. **Lighting Engine** (`gfx_lighting.c/h`)
   - Manages light sources (point, directional, spot)
   - Tracks lighting state
   - Provides API for adding/removing lights

2. **Shader Pack System** (`gfx_shader_pack.c/h`)
   - Loads custom shader packs from disk
   - Manages shader compilation
   - Minecraft Java Edition-style architecture

3. **OpenGL Backend** (`gfx_lighting_opengl.c/h`)
   - OpenGL-specific lighting implementation
   - Shadow map management
   - Shader compilation and uniform binding

4. **Lua API** (`smlua_lighting.c/h`)
   - Exposes lighting functions to Lua mods
   - Allows dynamic light creation/management
   - Shader pack control

### Integration Points

#### Configuration System
- Added 5 new config options in `configfile.h` and `configfile.c`
- Options persist in `sm64config.txt`

#### Renderer Pipeline
The lighting system hooks into the existing rendering pipeline without breaking compatibility:
- Original N64 color combiner system remains unchanged
- Lighting is applied as an optional post-process
- Can be toggled on/off at runtime

#### Build System
- Source files in `src/pc/gfx/` are automatically included
- No Makefile changes required (automatic discovery)

## File Structure

```
src/pc/gfx/
  gfx_lighting.h              # Core lighting API
  gfx_lighting.c              # Light management implementation
  gfx_lighting_opengl.h       # OpenGL backend API
  gfx_lighting_opengl.c       # OpenGL implementation
  gfx_shader_pack.h           # Shader pack system API
  gfx_shader_pack.c           # Shader pack loading/management

src/pc/lua/
  smlua_lighting.h            # Lua bindings header
  smlua_lighting.c            # Lua API implementation

src/pc/
  configfile.h                # Added lighting config options
  configfile.c                # Added lighting config variables

shaderpacks/
  default/                    # Default shader pack
    shaderpack.properties
    shaders/
      composite.vsh/fsh       # Main lighting shaders
      final.vsh/fsh           # Final pass shaders
      shadow.vsh/fsh          # Shadow generation shaders

docs/
  lighting_system.md          # User documentation

mods/
  lighting-example/           # Example Lua mod
    main.lua
```

## How It Works

### Initialization Flow

1. `gfx_lighting_init()` - Called during graphics init
2. `gfx_shader_pack_init()` - Initialize shader pack manager
3. `gfx_shader_pack_scan_directory("shaderpacks")` - Load available packs
4. If enabled, activate configured shader pack

### Rendering Flow

1. **Shadow Pass** (if enabled)
   - Render scene from each light's perspective
   - Generate depth maps

2. **Geometry Pass**
   - Render scene normally with N64 textures
   - Pass vertex normals and positions

3. **Lighting Pass**
   - Apply per-pixel lighting calculations
   - Sample shadow maps
   - Compute final lit color

4. **Final Pass**
   - Post-processing (tone mapping, etc.)
   - Output to framebuffer

### Light Management

Lights are stored in a global `LightingState` structure:
- Up to 32 lights supported
- Each light has type, position/direction, color, intensity
- Shadow maps allocated on-demand

## Shader Pack System

Similar to Minecraft Java Edition OptiFine/Iris shaders:

1. **Pack Discovery**: Scans `shaderpacks/` directory
2. **Properties Loading**: Reads `shaderpack.properties`
3. **Shader Compilation**: Compiles GLSL shaders
4. **Uniform Binding**: Connects lighting data to shader uniforms

### Shader Uniforms

Automatically provided to shaders:

```glsl
uniform int uLightingEnabled;
uniform int uNumLights;
uniform vec3 uAmbientLight;
uniform float uAmbientIntensity;
uniform Light uLights[32];
uniform sampler2D uShadowMap[8];
```

## Performance Considerations

### Optimizations

- Light culling (lights outside radius don't affect geometry)
- Deferred shadow map allocation
- Shader caching
- UBO support when available

### Scalability

- Configurable shadow quality (512, 1024, 2048, 4096)
- Per-light shadow toggle
- System can be completely disabled

## Compatibility

### N64 Mode
- Lighting system is PC-only
- Automatically disabled on N64 builds

### Existing Mods
- Does not interfere with existing rendering
- Lua mods can opt-in to lighting
- Falls back gracefully if disabled

### Platform Support

- **OpenGL 2.1+**: Core functionality
- **OpenGL 3.0+**: Shadow mapping, UBOs
- **OpenGL 3.3+**: Geometry shaders (for advanced effects)

## Future Extensions

Potential improvements:

1. **Direct3D Backend**: Currently OpenGL-only
2. **Area Lights**: Rectangle/disk light sources
3. **Light Probes**: Indirect lighting
4. **Cascaded Shadow Maps**: Better directional light shadows
5. **SSAO Integration**: Ambient occlusion
6. **PBR Materials**: Physical-based rendering

## Testing

To test the system:

1. Enable in config:
   ```
   lighting_enabled 1
   ```

2. Load example mod:
   ```
   Enable "lighting-example" mod
   ```

3. Verify lights appear in-game

4. Test shader packs:
   ```
   lighting_shader_packs_enabled 1
   lighting_active_shader_pack "default"
   ```

## Troubleshooting

### Lights not visible
- Check `configLightingEnabled` is true
- Verify OpenGL version (2.1+ required)
- Check light positions are valid

### Compilation errors
- Ensure GLEW is available
- Check OpenGL headers present
- Verify shader syntax (GLSL 120+)

### Shadow artifacts
- Increase shadow bias
- Reduce shadow distance
- Lower shadow quality if memory limited

## API Reference

See `docs/lighting_system.md` for complete Lua API documentation.

## Credits

Lighting system design inspired by:
- Minecraft Java Edition shader packs (OptiFine/Iris)
- Modern game engine lighting (Unity, Unreal)
- OpenGL deferred rendering techniques
