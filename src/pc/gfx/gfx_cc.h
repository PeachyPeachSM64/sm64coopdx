#ifndef GFX_CC_H
#define GFX_CC_H

#include <stdint.h>
#include <stdbool.h>

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

enum {
    SHADER_0,
    SHADER_INPUT_1,
    SHADER_INPUT_2,
    SHADER_INPUT_3,
    SHADER_INPUT_4,
    SHADER_INPUT_5,
    SHADER_INPUT_6,
    SHADER_INPUT_7,
    SHADER_INPUT_8,
    SHADER_TEXEL0,
    SHADER_TEXEL0A,
    SHADER_TEXEL1,
    SHADER_TEXEL1A,
    SHADER_1,
    SHADER_COMBINED,
    SHADER_COMBINEDA,
    SHADER_NOISE,
};

#define SHADER_MAX_INPUTS 8
#define SHADER_CMD_LENGTH 16
#define CC_MAX_SHADERS 64 // Keeping this here for now as it's needed for DirectX declarations

struct CCFeatures {
    bool used_textures[2];
    int num_inputs;
    bool do_single[4];
    bool do_multiply[4];
    bool do_mix[4];
    bool color_alpha_same[2];
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
        uint8_t all_values[SHADER_CMD_LENGTH];
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
    uint8_t shader_input_mapping[16];
    uint8_t shader_commands[SHADER_CMD_LENGTH];
};

#ifdef __cplusplus
extern "C" {
#endif

bool gfx_cm_uses_second_texture(struct CombineMode* cm);
void gfx_cc_get_features(struct ColorCombiner* cc, struct CCFeatures *cc_features);
void gfx_cc_print(struct ColorCombiner *cc);
uint32_t color_comb_rgb(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t cycle);
uint32_t color_comb_alpha(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t cycle);

#ifdef __cplusplus
}
#endif

#endif
