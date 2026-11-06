#ifndef GFX_SHADERS_H
#define GFX_SHADERS_H

#include "gfx_cc.h"
#include "gfx_pc.h"

//
// Attibutes
//

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
    enum VertexAttribType type;
    size_t offset;
    size_t count;
    size_t components;
    size_t stride;
};

struct VertexAttribs {
    float pos[4];
    float tex_coords[MAX_TEXTURES][2];
    struct {
        uint8_t primitive[4];
        uint8_t shade[4];
        uint8_t environment[4];
        uint8_t ambient[3];
        uint8_t lod;
    } inputs;
    uint8_t fog[4];
    uint16_t light_map[2];
};

const struct VertexAttribProperties *gfx_shaders_get_attrib_properties(size_t attrib);
size_t gfx_shaders_get_num_attribs();
void *gfx_shaders_get_attribs_data(const struct VertexAttribs *attribs, size_t count);
size_t gfx_shaders_get_attribs_stride();

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
    int ccs[CC_CYCLES][2];
    int ccf;
    float tex_properties[MAX_TEXTURES][3];
    float noise_frame_count;
    float light_map_color[3];
    int shader_flags;
    float shader_flag_values[SHADER_FLAG_MAX];
};

const struct ShaderUniformProperties *gfx_shaders_get_uniform_properties(enum ShaderUniform uniform);

#endif
