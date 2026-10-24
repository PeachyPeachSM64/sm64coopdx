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
    _Generic(((struct VertexAttribs *)0)->_member_, \
        int8_t:   ATTRIBUTE_TYPE_INT8, \
        uint8_t:  ATTRIBUTE_TYPE_UINT8, \
        int16_t:  ATTRIBUTE_TYPE_INT16, \
        uint16_t: ATTRIBUTE_TYPE_UINT16, \
        int32_t:  ATTRIBUTE_TYPE_INT32, \
        uint32_t: ATTRIBUTE_TYPE_UINT32, \
        float:    ATTRIBUTE_TYPE_FLOAT \
    )

#define vertex_attrib_single_comp(_member_)         vertex_attrib_type(_member_), single_comp(struct VertexAttribs, _member_)
#define vertex_attrib_multi_comp(_member_)          vertex_attrib_type(_member_[0]), multi_comp(struct VertexAttribs, _member_)
#define vertex_attrib_array_single_comp(_member_)   vertex_attrib_type(_member_[0]), array_single_comp(struct VertexAttribs, _member_)
#define vertex_attrib_array_multi_comp(_member_)    vertex_attrib_type(_member_[0][0]), array_multi_comp(struct VertexAttribs, _member_)

static const struct VertexAttribProperties VERTEX_ATTRIBS[] = {
    [ATTRIBUTE_POS]         = { "aVtxPos",      false, vertex_attrib_multi_comp(pos)                },
    [ATTRIBUTE_TEX_COORDS]  = { "aTexCoords",   false, vertex_attrib_array_multi_comp(tex_coords)   },
    [ATTRIBUTE_PRIMITIVE]   = { "aPrimitive",   true,  vertex_attrib_multi_comp(inputs.primitive)   },
    [ATTRIBUTE_SHADE]       = { "aShade",       true,  vertex_attrib_multi_comp(inputs.shade)       },
    [ATTRIBUTE_ENVIRONMENT] = { "aEnvironment", true,  vertex_attrib_multi_comp(inputs.environment) },
    [ATTRIBUTE_LOD]         = { "aLod",         true,  vertex_attrib_multi_comp(inputs.lod)         },
    [ATTRIBUTE_FOG]         = { "aFog",         true,  vertex_attrib_multi_comp(fog)                },
    [ATTRIBUTE_LIGHT_MAP]   = { "aLightMap",    true,  vertex_attrib_multi_comp(light_map)          },
};

const struct VertexAttribProperties *gfx_shaders_get_attrib_properties(enum VertexAttrib attrib) {
    return &VERTEX_ATTRIBS[attrib];
}

//
// Uniforms
//

#define shader_uniform_single_comp(_member_)        single_comp(struct ShaderUniforms, _member_)
#define shader_uniform_multi_comp(_member_)         multi_comp(struct ShaderUniforms, _member_)
#define shader_uniform_array_single_comp(_member_)  array_single_comp(struct ShaderUniforms, _member_)
#define shader_uniform_array_multi_comp(_member_)   array_multi_comp(struct ShaderUniforms, _member_)

static const struct ShaderUniformProperties SHADER_UNIFORMS[] = {
    [UNIFORM_CCS]                = { "uCCS",              shader_uniform_array_single_comp(ccs)                },
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



