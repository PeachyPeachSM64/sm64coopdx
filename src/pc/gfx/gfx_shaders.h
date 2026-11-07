#ifndef GFX_SHADERS_H
#define GFX_SHADERS_H

#include "gfx_cc.h"
#include "gfx_pc.h"

enum ShaderDataType {
    SHADER_DATA_TYPE_INT8,
    SHADER_DATA_TYPE_UINT8,
    SHADER_DATA_TYPE_INT16,
    SHADER_DATA_TYPE_UINT16,
    SHADER_DATA_TYPE_INT32,
    SHADER_DATA_TYPE_UINT32,
    SHADER_DATA_TYPE_FLOAT,
};

struct ShaderDataProperties {
    const char *name;
    enum ShaderDataType type;
    size_t offset;
    size_t count;
    size_t components;
    size_t stride;
};

//
// Attibutes
//

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

const struct ShaderDataProperties *gfx_shaders_get_attrib_properties(size_t attrib);
size_t gfx_shaders_get_num_attribs();
void *gfx_shaders_get_attribs_data(const struct VertexAttribs *attribs, size_t count);
size_t gfx_shaders_get_attribs_stride();

//
// Uniforms
//

struct ShaderUniforms {
    uint8_t ccs[CC_CYCLES][2][CC_COMPONENTS];
    uint32_t ccf;
    struct {
        uint32_t width;
        uint32_t height;
        uint8_t filter;
    } tex_properties[MAX_TEXTURES];
    uint32_t noise_random_seed;
    uint8_t light_map_color[3];
    uint32_t shader_flags;
    float shader_flag_values[SHADER_FLAG_MAX];
    bool dirty;
};

const struct ShaderDataProperties *gfx_shaders_get_uniform_properties(size_t uniform);
size_t gfx_shaders_get_num_uniforms();
void *gfx_shaders_get_uniforms_data(const struct ShaderUniforms *uniforms);
size_t gfx_shaders_get_uniforms_stride();
void gfx_shaders_update_uniforms(struct ShaderUniforms *uniforms, const struct ColorCombiner *cc, const struct TextureProperties *texture, uint8_t tile);

#endif
