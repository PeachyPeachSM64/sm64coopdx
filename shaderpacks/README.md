# Shader Packs

This directory contains shader packs for the optional lighting system.

## Directory Structure

Each shader pack should be in its own subdirectory:

```
shaderpacks/
  default/              # Built-in default lighting shader
  my-custom-pack/       # Your custom shader pack
    shaderpack.properties
    shaders/
      composite.vsh
      composite.fsh
      final.vsh
      final.fsh
      shadow.vsh
      shadow.fsh
```

## Creating a Shader Pack

1. Create a new directory in `shaderpacks/`
2. Add `shaderpack.properties` with metadata
3. Create a `shaders/` subdirectory
4. Add shader files (see default pack for examples)

## Shader Files

- **composite.vsh/fsh**: Main lighting calculations
- **final.vsh/fsh**: Post-processing and final output
- **shadow.vsh/fsh**: Shadow map generation (optional)

## Loading Shader Packs

Shader packs are automatically scanned on startup. Enable them via:
- Config file: `lighting_shader_packs_enabled 1`
- Lua: `shader_pack_set_enabled(true)`

Activate a specific pack:
- Config file: `lighting_active_shader_pack "pack-name"`
- Lua: `shader_pack_activate(index)`

See `docs/lighting_system.md` for full documentation.
