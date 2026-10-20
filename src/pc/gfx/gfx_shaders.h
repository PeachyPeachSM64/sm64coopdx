#ifndef GFX_SHADERS_H
#define GFX_SHADERS_H

#include "gfx_cc.h"
#include "gfx_pc.h"

//
// Attibutes
//

enum VertexAttrib {
    ATTRIBUTE_POS,
    ATTRIBUTE_TEX_COORDS,
    ATTRIBUTE_INPUTS,
    ATTRIBUTE_FOG,
    ATTRIBUTE_LIGHT_MAP,
    MAX_ATTRIBUTES,
};

enum VertexAttribType {
    ATTRIBUTE_TYPE_INT8,
    ATTRIBUTE_TYPE_UINT8,
    ATTRIBUTE_TYPE_INT16,
    ATTRIBUTE_TYPE_UINT16,
    ATTRIBUTE_TYPE_INT32,
    ATTRIBUTE_TYPE_UINT32,
    ATTRIBUTE_TYPE_FLOAT,
};

struct VertexAttribProperties {
    const char *name;
    bool normalized;
    enum VertexAttribType type;
    size_t offset;
    size_t count;
    size_t components;
    size_t stride;
};

struct VertexAttribs {
    float pos[4];
    float tex_coords[MAX_TEXTURES][2];
    uint8_t inputs[MAX_SHADER_INPUTS][4];
    uint8_t fog[4];
    uint16_t light_map[2];
};

const struct VertexAttribProperties *gfx_shaders_get_attrib_properties(enum VertexAttrib attrib);

//
// Uniforms
//

enum ShaderUniform {
    UNIFORM_CCS,
    UNIFORM_CCF,
    UNIFORM_TEX_PROPERTIES,
    UNIFORM_NOISE_FRAME_COUNT,
    UNIFORM_LIGHT_MAP_COLOR,
    UNIFORM_SHADER_FLAGS,
    UNIFORM_SHADER_FLAG_VALUES,
    MAX_UNIFORMS,
};

struct ShaderUniformProperties {
    const char *name;
    size_t offset;
    size_t count;
    size_t components;
    size_t stride;
};

struct ShaderUniforms {
    int ccs[CC_CHANNELS * CC_CYCLES];
    int ccf;
    float tex_properties[MAX_TEXTURES][3];
    float noise_frame_count;
    float light_map_color[3];
    int shader_flags;
    float shader_flag_values[SHADER_FLAG_MAX];
};

const struct ShaderUniformProperties *gfx_shaders_get_uniform_properties(enum ShaderUniform uniform);

#endif
