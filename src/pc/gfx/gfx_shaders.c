#include "gfx_shaders.h"

#define tsizeof(_type_, _member_) sizeof(((_type_ *)0)->_member_)
#define countof(_type_, _member_) (tsizeof(_type_, _member_) / tsizeof(_type_, _member_[0]))

#define single_comp(_type_, _member_) \
    .offset = offsetof(_type_, _member_), \
    .count = 1, \
    .components = 1, \
    .stride = tsizeof(_type_, _member_)

#define multi_comp(_type_, _member_) \
    .offset = offsetof(_type_, _member_), \
    .count = 1, \
    .components = countof(_type_, _member_), \
    .stride = tsizeof(_type_, _member_)

#define array_single_comp(_type_, _member_) \
    .offset = offsetof(_type_, _member_), \
    .count = countof(_type_, _member_), \
    .components = 1, \
    .stride = tsizeof(_type_, _member_[0])

#define array_multi_comp(_type_, _member_) \
    .offset = offsetof(_type_, _member_), \
    .count = countof(_type_, _member_), \
    .components = countof(_type_, _member_[0]), \
    .stride = tsizeof(_type_, _member_[0])

//
// Attibutes
//

#define vertex_attrib_type(_member_) \
    _Generic(((struct VertexAttribsData *)0)->_member_, \
        int8_t:   ATTRIBUTE_TYPE_INT8, \
        uint8_t:  ATTRIBUTE_TYPE_UINT8, \
        int16_t:  ATTRIBUTE_TYPE_INT16, \
        uint16_t: ATTRIBUTE_TYPE_UINT16, \
        int32_t:  ATTRIBUTE_TYPE_INT32, \
        uint32_t: ATTRIBUTE_TYPE_UINT32, \
        float:    ATTRIBUTE_TYPE_FLOAT \
    )

#define vertex_attrib_single_comp(_member_)         vertex_attrib_type(_member_), single_comp(struct VertexAttribsData, _member_)
#define vertex_attrib_multi_comp(_member_)          vertex_attrib_type(_member_[0]), multi_comp(struct VertexAttribsData, _member_)
#define vertex_attrib_array_single_comp(_member_)   vertex_attrib_type(_member_[0]), array_single_comp(struct VertexAttribsData, _member_)
#define vertex_attrib_array_multi_comp(_member_)    vertex_attrib_type(_member_[0][0]), array_multi_comp(struct VertexAttribsData, _member_)

enum VertexAttrib {
    ATTRIBUTE_POS,
    ATTRIBUTE_TEX_COORDS,
    ATTRIBUTE_INPUTS,
    ATTRIBUTE_FEATURES,
    MAX_ATTRIBUTES,
};

struct VertexAttribsData {
    float pos[4];
    float tex_coords[MAX_TEXTURES / 2][4];
    int inputs[4];
    int features[4];
};

static const struct VertexAttribProperties VERTEX_ATTRIBS[] = {
    [ATTRIBUTE_POS]        = { "aVtxPos",    vertex_attrib_multi_comp(pos)              },
    [ATTRIBUTE_TEX_COORDS] = { "aTexCoords", vertex_attrib_array_multi_comp(tex_coords) },
    [ATTRIBUTE_INPUTS]     = { "aInputs",    vertex_attrib_multi_comp(inputs)           },
    [ATTRIBUTE_FEATURES]   = { "aFeatures",  vertex_attrib_multi_comp(features)         },
};

const struct VertexAttribProperties *gfx_shaders_get_attrib_properties(size_t attrib) {
    if (attrib >= MAX_ATTRIBUTES) { return NULL; }
    return &VERTEX_ATTRIBS[attrib];
}

size_t gfx_shaders_get_num_attribs() {
    return MAX_ATTRIBUTES;
}

static struct VertexAttribsData gfx_shaders_pack_attribs(const struct VertexAttribs *attribs) {
    struct VertexAttribsData data = {0};

    // Pos
    data.pos[0] = attribs->pos[0];
    data.pos[1] = attribs->pos[1];
    data.pos[2] = attribs->pos[2];
    data.pos[3] = attribs->pos[3];

    // Tex coords
    for (size_t i = 0; i < MAX_TEXTURES; ++i) {
        data.tex_coords[i / 2][2 * (i % 2) + 0] = attribs->tex_coords[i][0];
        data.tex_coords[i / 2][2 * (i % 2) + 1] = attribs->tex_coords[i][1];
    }

    // Shader inputs
    data.inputs[0] = (
        _SHIFTL(attribs->inputs.primitive[0], 24, 8) |
        _SHIFTL(attribs->inputs.primitive[1], 16, 8) |
        _SHIFTL(attribs->inputs.primitive[2],  8, 8) |
        _SHIFTL(attribs->inputs.primitive[3],  0, 8)
    );
    data.inputs[1] = (
        _SHIFTL(attribs->inputs.shade[0], 24, 8) |
        _SHIFTL(attribs->inputs.shade[1], 16, 8) |
        _SHIFTL(attribs->inputs.shade[2],  8, 8) |
        _SHIFTL(attribs->inputs.shade[3],  0, 8)
    );
    data.inputs[2] = (
        _SHIFTL(attribs->inputs.environment[0], 24, 8) |
        _SHIFTL(attribs->inputs.environment[1], 16, 8) |
        _SHIFTL(attribs->inputs.environment[2],  8, 8) |
        _SHIFTL(attribs->inputs.environment[3],  0, 8)
    );
    data.inputs[3] = (
        _SHIFTL(attribs->inputs.ambient[0], 24, 8) |
        _SHIFTL(attribs->inputs.ambient[1], 16, 8) |
        _SHIFTL(attribs->inputs.ambient[2],  8, 8) |
        _SHIFTL(attribs->inputs.lod,         0, 8)
    );

    // Features
    data.features[0] = (
        _SHIFTL(attribs->fog[0], 24, 8) |
        _SHIFTL(attribs->fog[1], 16, 8) |
        _SHIFTL(attribs->fog[2],  8, 8) |
        _SHIFTL(attribs->fog[3],  0, 8)
    );
    data.features[1] = (
        _SHIFTL(attribs->light_map[0], 16, 16) |
        _SHIFTL(attribs->light_map[1],  0, 16)
    );

    return data;
}

void *gfx_shaders_get_attribs_data(const struct VertexAttribs *attribs, size_t count) {
    static struct VertexAttribsData sVertexAttribsData[MAX_BUFFERED_VERTS];
    if (!attribs || count > MAX_BUFFERED_VERTS) { return NULL; }

    for (size_t i = 0; i < count; ++i) {
        sVertexAttribsData[i] = gfx_shaders_pack_attribs(attribs + i);
    }

    return sVertexAttribsData;
}

size_t gfx_shaders_get_attribs_stride() {
    return sizeof(struct VertexAttribsData);
}

//
// Uniforms
//

#define shader_uniform_single_comp(_member_)        single_comp(struct ShaderUniforms, _member_)
#define shader_uniform_multi_comp(_member_)         multi_comp(struct ShaderUniforms, _member_)
#define shader_uniform_array_single_comp(_member_)  array_single_comp(struct ShaderUniforms, _member_)
#define shader_uniform_array_multi_comp(_member_)   array_multi_comp(struct ShaderUniforms, _member_)

static const struct ShaderUniformProperties SHADER_UNIFORMS[] = {
    [UNIFORM_CCS]                = { "uCCS",              shader_uniform_array_multi_comp(ccs)                 },
    [UNIFORM_CCF]                = { "uCCF",              shader_uniform_single_comp(ccf)                      },
    [UNIFORM_TEX_PROPERTIES]     = { "uTexProperties",    shader_uniform_array_multi_comp(tex_properties)      },
    [UNIFORM_NOISE_FRAME_COUNT]  = { "uNoiseFrameCount",  shader_uniform_single_comp(noise_frame_count)        },
    [UNIFORM_LIGHT_MAP_COLOR]    = { "uLightMapColor",    shader_uniform_multi_comp(light_map_color)           },
    [UNIFORM_SHADER_FLAGS]       = { "uShaderFlags",      shader_uniform_single_comp(shader_flags)             },
    [UNIFORM_SHADER_FLAG_VALUES] = { "uShaderFlagValues", shader_uniform_array_single_comp(shader_flag_values) },
};

const struct ShaderUniformProperties *gfx_shaders_get_uniform_properties(enum ShaderUniform uniform) {
    return &SHADER_UNIFORMS[uniform];
}



