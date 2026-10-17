#ifndef GFX_SHADERS_H
#define GFX_SHADERS_H

#include "gfx_cc.h"

enum {
    ATTRIBUTE_POS,
    ATTRIBUTE_TEX_COORDS,
    ATTRIBUTE_INPUTS,
    ATTRIBUTE_FOG,
    ATTRIBUTE_LIGHT_MAP,
    MAX_ATTRIBUTES,
};

enum {
    UNIFORM_CCS,
    UNIFORM_CCF,
    UNIFORM_FRAME_COUNT,
    UNIFORM_LIGHT_MAP,
    UNIFORM_SHADER_FLAGS,
    UNIFORM_SHADER_FLAG_VALUES,
    UNIFORM_TEX_PROPERTIES,
    MAX_UNIFORMS,
};

struct VertexAttribs {
    float pos[4];
    float tex_coords[MAX_TEXTURES][2];
    uint8_t fog[4];
    uint16_t light_map[2];
    uint8_t inputs[MAX_SHADER_INPUTS][4];
};

#endif
