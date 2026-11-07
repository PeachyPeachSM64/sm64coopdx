#include "gfx_shaders.h"

#include "game/game_init.h"

#define tsizeof(_type_, _member_) sizeof(((_type_ *)0)->_member_)
#define countof(_type_, _member_) (tsizeof(_type_, _member_) / tsizeof(_type_, _member_[0]))

#define shader_data_type(_type_, _member_) \
    _Generic(((_type_ *)0)->_member_, \
        int8_t:   SHADER_DATA_TYPE_INT8, \
        uint8_t:  SHADER_DATA_TYPE_UINT8, \
        int16_t:  SHADER_DATA_TYPE_INT16, \
        uint16_t: SHADER_DATA_TYPE_UINT16, \
        int32_t:  SHADER_DATA_TYPE_INT32, \
        uint32_t: SHADER_DATA_TYPE_UINT32, \
        float:    SHADER_DATA_TYPE_FLOAT \
    )

// float x
#define single_comp(_type_, _member_) \
    .type = shader_data_type(_type_, _member_), \
    .offset = offsetof(_type_, _member_), \
    .count = 1, \
    .components = 1, \
    .stride = tsizeof(_type_, _member_)

// float4 x
#define multi_comp(_type_, _member_) \
    .type = shader_data_type(_type_, _member_[0]), \
    .offset = offsetof(_type_, _member_), \
    .count = 1, \
    .components = countof(_type_, _member_), \
    .stride = tsizeof(_type_, _member_)

// float x[2]
#define array_single_comp(_type_, _member_) \
    .type = shader_data_type(_type_, _member_[0]), \
    .offset = offsetof(_type_, _member_), \
    .count = countof(_type_, _member_), \
    .components = 1, \
    .stride = tsizeof(_type_, _member_[0])

// float4 x[2]
#define array_multi_comp(_type_, _member_) \
    .type = shader_data_type(_type_, _member_[0][0]), \
    .offset = offsetof(_type_, _member_), \
    .count = countof(_type_, _member_), \
    .components = countof(_type_, _member_[0]), \
    .stride = tsizeof(_type_, _member_[0])

//
// Attibutes
//

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

// Everything must stay aligned to 16-byte boundaries
static_assert(offsetof(struct VertexAttribsData, pos) % 16 == 0);
static_assert(offsetof(struct VertexAttribsData, tex_coords) % 16 == 0);
static_assert(offsetof(struct VertexAttribsData, inputs) % 16 == 0);
static_assert(offsetof(struct VertexAttribsData, features) % 16 == 0);
static_assert(sizeof(struct VertexAttribsData) % 16 == 0);

static const struct ShaderDataProperties VERTEX_ATTRIBS[] = {
    [ATTRIBUTE_POS]        = { "aVtxPos",    multi_comp(struct VertexAttribsData, pos)              },
    [ATTRIBUTE_TEX_COORDS] = { "aTexCoords", array_multi_comp(struct VertexAttribsData, tex_coords) },
    [ATTRIBUTE_INPUTS]     = { "aInputs",    multi_comp(struct VertexAttribsData, inputs)           },
    [ATTRIBUTE_FEATURES]   = { "aFeatures",  multi_comp(struct VertexAttribsData, features)         },
};

const struct ShaderDataProperties *gfx_shaders_get_attrib_properties(size_t attrib) {
    if (attrib >= MAX_ATTRIBUTES) { return NULL; }
    return &VERTEX_ATTRIBS[attrib];
}

size_t gfx_shaders_get_num_attribs() {
    return MAX_ATTRIBUTES;
}

static void gfx_shaders_pack_attribs(struct VertexAttribsData *data, const struct VertexAttribs *attribs) {
    memset(data, 0, sizeof(*data));

    // Pos
    data->pos[0] = attribs->pos[0];
    data->pos[1] = attribs->pos[1];
    data->pos[2] = attribs->pos[2];
    data->pos[3] = attribs->pos[3];

    // Tex coords
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        data->tex_coords[i / 2][2 * (i % 2) + 0] = attribs->tex_coords[i][0];
        data->tex_coords[i / 2][2 * (i % 2) + 1] = attribs->tex_coords[i][1];
    }

    // Shader inputs
    data->inputs[0] = (
        _SHIFTL(attribs->inputs.primitive[0], 24, 8) |
        _SHIFTL(attribs->inputs.primitive[1], 16, 8) |
        _SHIFTL(attribs->inputs.primitive[2],  8, 8) |
        _SHIFTL(attribs->inputs.primitive[3],  0, 8)
    );
    data->inputs[1] = (
        _SHIFTL(attribs->inputs.shade[0], 24, 8) |
        _SHIFTL(attribs->inputs.shade[1], 16, 8) |
        _SHIFTL(attribs->inputs.shade[2],  8, 8) |
        _SHIFTL(attribs->inputs.shade[3],  0, 8)
    );
    data->inputs[2] = (
        _SHIFTL(attribs->inputs.environment[0], 24, 8) |
        _SHIFTL(attribs->inputs.environment[1], 16, 8) |
        _SHIFTL(attribs->inputs.environment[2],  8, 8) |
        _SHIFTL(attribs->inputs.environment[3],  0, 8)
    );
    data->inputs[3] = (
        _SHIFTL(attribs->inputs.ambient[0], 24, 8) |
        _SHIFTL(attribs->inputs.ambient[1], 16, 8) |
        _SHIFTL(attribs->inputs.ambient[2],  8, 8) |
        _SHIFTL(attribs->inputs.lod,         0, 8)
    );

    // Features
    data->features[0] = (
        _SHIFTL(attribs->fog[0], 24, 8) |
        _SHIFTL(attribs->fog[1], 16, 8) |
        _SHIFTL(attribs->fog[2],  8, 8) |
        _SHIFTL(attribs->fog[3],  0, 8)
    );
    data->features[1] = (
        _SHIFTL(attribs->light_map[0], 16, 16) |
        _SHIFTL(attribs->light_map[1],  0, 16)
    );
}

void *gfx_shaders_get_attribs_data(const struct VertexAttribs *attribs, size_t count) {
    static struct VertexAttribsData sVertexAttribsData[MAX_BUFFERED_VERTS];
    if (!attribs || count > MAX_BUFFERED_VERTS) { return NULL; }

    for (size_t i = 0; i < count; ++i) {
        gfx_shaders_pack_attribs(sVertexAttribsData + i, attribs + i);
    }

    return sVertexAttribsData;
}

size_t gfx_shaders_get_attribs_stride() {
    return sizeof(struct VertexAttribsData);
}

//
// Uniforms
//

enum ShaderUniform {
    UNIFORM_CCS,
    UNIFORM_TEX_PROPERTIES,
    UNIFORM_FEATURES,
    UNIFORM_SHADER_FLAG_VALUES,
    MAX_UNIFORMS,
};

struct ShaderUniformsData {
    int ccs[CC_CYCLES / 2][4];
    int tex_properties[MAX_TEXTURES][4]; // width, height, filter
    int features[4]; // ccf, noise_random_seed, light_map_color, shader_flags
    float shader_flag_values[(SHADER_FLAG_MAX + 3) / 4][4];
};

// Everything must stay aligned to 16-byte boundaries
static_assert(offsetof(struct ShaderUniformsData, ccs) % 16 == 0);
static_assert(offsetof(struct ShaderUniformsData, tex_properties) % 16 == 0);
static_assert(offsetof(struct ShaderUniformsData, features) % 16 == 0);
static_assert(offsetof(struct ShaderUniformsData, shader_flag_values) % 16 == 0);
static_assert(sizeof(struct ShaderUniformsData) % 16 == 0);

static const struct ShaderDataProperties SHADER_UNIFORMS[] = {
    [UNIFORM_CCS]                = { "uCCS",              array_multi_comp(struct ShaderUniformsData, ccs)                },
    [UNIFORM_TEX_PROPERTIES]     = { "uTexProperties",    array_multi_comp(struct ShaderUniformsData, tex_properties)     },
    [UNIFORM_FEATURES]           = { "uFeatures",         multi_comp(struct ShaderUniformsData, features)                 },
    [UNIFORM_SHADER_FLAG_VALUES] = { "uShaderFlagValues", array_multi_comp(struct ShaderUniformsData, shader_flag_values) },
};

const struct ShaderDataProperties *gfx_shaders_get_uniform_properties(size_t uniform) {
    if (uniform >= MAX_UNIFORMS) { return NULL; }
    return &SHADER_UNIFORMS[uniform];
}

size_t gfx_shaders_get_num_uniforms() {
    return MAX_UNIFORMS;
}

static void gfx_shaders_pack_uniforms(struct ShaderUniformsData *data, const struct ShaderUniforms *uniforms) {
    memset(data, 0, sizeof(*data));

    // CC sources
    for (int cycle = 0; cycle < CC_CYCLES; ++cycle)
    for (int i = 0; i < 2; ++i) {
        data->ccs[cycle / 2][2 * (cycle % 2) + i] = (
            _SHIFTL(uniforms->ccs[cycle][i][0], 24, 8) |
            _SHIFTL(uniforms->ccs[cycle][i][1], 16, 8) |
            _SHIFTL(uniforms->ccs[cycle][i][2],  8, 8) |
            _SHIFTL(uniforms->ccs[cycle][i][3],  0, 8)
        );
    }

    // Tex properties
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        data->tex_properties[i][0] = uniforms->tex_properties[i].width;
        data->tex_properties[i][1] = uniforms->tex_properties[i].height;
        data->tex_properties[i][2] = uniforms->tex_properties[i].filter;
    }

    // Features
    data->features[0] = uniforms->ccf;
    data->features[1] = uniforms->noise_random_seed;
    data->features[2] = (
        _SHIFTL(uniforms->light_map_color[0], 24, 8) |
        _SHIFTL(uniforms->light_map_color[1], 16, 8) |
        _SHIFTL(uniforms->light_map_color[2],  8, 8) |
        0xFF
    );
    data->features[3] = uniforms->shader_flags;

    // Shader flag values
    for (int i = 0; i < SHADER_FLAG_MAX; ++i) {
        data->shader_flag_values[i / 4][i % 4] = uniforms->shader_flag_values[i];
    }
}

void *gfx_shaders_get_uniforms_data(const struct ShaderUniforms *uniforms) {
    static struct ShaderUniformsData sShaderUniformsData;
    if (!uniforms) { return NULL; }

    gfx_shaders_pack_uniforms(&sShaderUniformsData, uniforms);

    return &sShaderUniformsData;
}

size_t gfx_shaders_get_uniforms_stride() {
    return sizeof(struct ShaderUniformsData);
}

void gfx_shaders_update_uniforms(struct ShaderUniforms *uniforms, const struct ColorCombiner *cc, const struct TextureProperties *texture, uint8_t tile) {
    if (!uniforms || (!cc && (!texture || tile >= MAX_TEXTURES))) { return; }

    // CC sources
    if (cc) {
        memcpy(uniforms->ccs, cc->ccs, sizeof(uniforms->ccs));
    }

    // CC features
    if (cc) {
        uniforms->ccf = 0;
        uniforms->ccf |= (cc->use_noise << CCF_NOISE);
        uniforms->ccf |= (cc->use_alpha << CCF_USE_ALPHA);
        uniforms->ccf |= (cc->use_fog << CCF_USE_FOG);
        uniforms->ccf |= (cc->texture_edge << CCF_TEX_EDGE);
        uniforms->ccf |= (cc->use_dither << CCF_USE_DITHER);
        uniforms->ccf |= (cc->use_2cycle << CCF_USE_2CYCLE);
        uniforms->ccf |= (cc->light_map << CCF_LIGHT_MAP);
        uniforms->ccf |= (cc->world_geometry << CCF_WORLD_GEOMETRY);
        uniforms->ccf |= (cc->use_ext_color << CCF_USE_EXT_COLOR);
        uniforms->ccf |= (cc->use_ext_alpha << CCF_USE_EXT_ALPHA);
        uniforms->ccf |= (cc->ext_color_alpha_same << CCF_EXT_COLOR_ALPHA_SAME);
        uniforms->ccf |= (cc->used_textures << CCF_USED_TEXTURE);
    }

    // Tex properties
    if (texture) {
        uniforms->tex_properties[tile].width = texture->width;
        uniforms->tex_properties[tile].height = texture->height;
        uniforms->tex_properties[tile].filter = texture->filter;
    }

    // Noise
    if (cc && ((cc->use_alpha && cc->use_dither) || cc->use_noise)) {
        uniforms->noise_random_seed = gGlobalTimer;
    }

    // Light map
    if (cc && cc->light_map) {
        uniforms->light_map_color[0] = gVertexColor[0];
        uniforms->light_map_color[1] = gVertexColor[1];
        uniforms->light_map_color[2] = gVertexColor[2];
    }

    // Global shader effects
    if (cc && cc->world_geometry) {
        uniforms->shader_flags = 0;
        for (int i = 0; i < SHADER_FLAG_MAX; ++i) {
            uniforms->shader_flags |= (gShaderFlags[i] << i);
            uniforms->shader_flag_values[i] = gShaderFlagValues[i];
        }
    }

    uniforms->dirty = true;
}
