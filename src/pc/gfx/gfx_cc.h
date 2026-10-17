#ifndef GFX_CC_H
#define GFX_CC_H

#include <stdint.h>
#include <stdbool.h>
#include "gfx.h"

#include "gfx_cc.inl"

// Make sure the type for shader_inputs can hold all shader inputs
typedef uint8_t shader_inputs_t;
static_assert(sizeof(shader_inputs_t) * 8 >= MAX_SHADER_INPUTS, "Type for shader_inputs is not large enough to hold all MAX_SHADER_INPUTS shader inputs");

#pragma pack(1)
struct ColorCombiner {
    uint8_t ccs[CC_CYCLES * CC_CHANNELS * CC_COMPONENTS];
    shader_inputs_t shader_inputs;
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
        uint8_t flags;
    };
    union {
        struct {
            uint8_t use_noise     : 1;
            uint8_t used_textures : MAX_TEXTURES;
        };
        uint8_t features;
    };
};
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

void gfx_cc_generate(struct ColorCombiner *cc,
    uint8_t  a0, uint8_t  b0, uint8_t  c0, uint8_t  d0,
    uint8_t Aa0, uint8_t Ab0, uint8_t Ac0, uint8_t Ad0,
    uint8_t  a1, uint8_t  b1, uint8_t  c1, uint8_t  d1,
    uint8_t Aa1, uint8_t Ab1, uint8_t Ac1, uint8_t Ad1
);
void gfx_cc_update_features(struct ColorCombiner *cc);

#ifdef __cplusplus
}
#endif

#endif
