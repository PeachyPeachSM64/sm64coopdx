#ifndef GFX_CC_H
#define GFX_CC_H

#include <stdint.h>
#include <stdbool.h>
#include "gfx.h"

#include "gfx_cc.inl"

// Make sure CC_COMPONENTS is 4, otherwise it breaks the aligment of struct CombineMode
static_assert(CC_COMPONENTS == sizeof(uint32_t), "CC_COMPONENTS must be sizeof(uint32_t)");

// Make sure the type for shader_inputs can hold all shader inputs
typedef uint8_t shader_inputs_t;
static_assert(sizeof(shader_inputs_t) * 8 >= MAX_SHADER_INPUTS, "Type for shader_inputs is not large enough to hold all MAX_SHADER_INPUTS shader inputs");

#pragma pack(1)
struct CombineMode {
    union {
        struct {
            uint32_t rgb1;
            uint32_t alpha1;
            uint32_t rgb2;
            uint32_t alpha2;
        };
        struct {
            uint32_t rgb;
            uint32_t alpha;
        } cycles[CC_CYCLES];
        uint8_t all_values[CC_COMPONENTS * CC_CHANNELS * CC_CYCLES];
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
    shader_inputs_t shader_inputs;
    bool used_textures[MAX_TEXTURES];
    bool color_alpha_same[CC_CYCLES];
    bool do_single[CC_CHANNELS * CC_CYCLES];       // TODO: optimize this shit, those booleans are mutually exclusive
    bool do_multiply[CC_CHANNELS * CC_CYCLES];     // TODO: optimize this shit, those booleans are mutually exclusive
    bool do_mix[CC_CHANNELS * CC_CYCLES];          // TODO: optimize this shit, those booleans are mutually exclusive
    bool do_noise;
};

struct VertexAttribs {
    float pos[4];
    float tex_coords[MAX_TEXTURES][2];
    uint8_t fog[4];
    uint16_t light_map[2];
    uint8_t inputs[MAX_SHADER_INPUTS][4];
};

#ifdef __cplusplus
extern "C" {
#endif

bool gfx_cm_uses_second_texture(struct CombineMode *cm);
bool gfx_cc_generate(struct ColorCombiner *cc, struct CombineMode *cm);
uint32_t color_comb(bool alpha, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t cycle);

#ifdef __cplusplus
}
#endif

#endif
