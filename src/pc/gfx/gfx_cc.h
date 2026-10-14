#ifndef GFX_CC_H
#define GFX_CC_H

#include <stdint.h>
#include <stdbool.h>
#include "gfx.h"

#include "gfx_cc.inl"

#define CC_MAX_SHADERS 64 // Keeping this here for now as it's needed for DirectX declarations

enum {
    CC_0,
    CC_TEXEL0,
    CC_TEXEL1,
    CC_PRIM,
    CC_SHADE,
    CC_ENV,
    CC_TEXEL0A,
    CC_LOD,
    CC_1,
    CC_TEXEL1A,
    CC_COMBINED,
    CC_COMBINEDA,
    CC_PRIMA,
    CC_SHADEA,
    CC_ENVA,
    CC_NOISE,
    CC_ENUM_MAX,
};

struct CCFeatures {
    int num_inputs;
    bool used_textures[MAX_TEXTURES];
    bool color_alpha_same[CC_CYCLES];
    bool do_single[CC_MODES];       // TODO: optimize this shit, those booleans are mutually exclusive
    bool do_multiply[CC_MODES];     // TODO: optimize this shit, those booleans are mutually exclusive
    bool do_mix[CC_MODES];          // TODO: optimize this shit, those booleans are mutually exclusive
    bool do_noise;
};

#pragma pack(1)
struct CombineMode {
    union {
        struct {
            uint32_t rgb1;
            uint32_t alpha1;
            uint32_t rgb2;
            uint32_t alpha2;
        };
        uint8_t all_values[CC_CMD_LENGTH];
    };
    union {
        struct {
            uint8_t use_alpha      : 1;
            uint8_t use_fog        : 1;
            uint8_t texture_edge   : 1;
            uint8_t use_dither     : 1;
            uint8_t use_2cycle     : 1;
            uint8_t light_map      : 1;
            uint8_t world_geometry : 1;
        };
        uint32_t flags;
    };
    uint64_t hash;
};
#pragma pack()

struct ColorCombiner {
    struct CombineMode cm;
    uint8_t shader_input_mapping[SHADER_MAX_INPUTS];
    uint8_t shader_commands[CC_CMD_LENGTH];
};

struct VertexAttribs {
    float pos[4];
    float tex_coords[MAX_TEXTURES][2];
    uint8_t fog[4];
    uint16_t light_map[2];
    uint8_t inputs[SHADER_MAX_INPUTS][4];
};

#ifdef __cplusplus
extern "C" {
#endif

bool gfx_cm_uses_second_texture(struct CombineMode *cm);
void gfx_cc_generate_shader_commands(struct ColorCombiner *cc);
void gfx_cc_get_features(struct ColorCombiner *cc, struct CCFeatures *ccf);
void gfx_cc_print(struct ColorCombiner *cc);
uint32_t color_comb_rgb(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t cycle);
uint32_t color_comb_alpha(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t cycle);

#ifdef __cplusplus
}
#endif

#endif
