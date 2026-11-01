#ifndef GFX_CC_H
#define GFX_CC_H

#include <stdint.h>
#include <stdbool.h>
#include "gfx.h"

#include "gfx_cc.inl"
#include "gfx_cc_ext.h"

#define SHADER_INPUT_PRIMITIVE      (1 << 0)
#define SHADER_INPUT_SHADE          (1 << 1)
#define SHADER_INPUT_ENVIRONMENT    (1 << 2)
#define SHADER_INPUT_LOD            (1 << 3)

#define SHADER_INPUT_BITS (0 \
    | SHADER_INPUT_PRIMITIVE \
    | SHADER_INPUT_SHADE \
    | SHADER_INPUT_ENVIRONMENT \
    | SHADER_INPUT_LOD \
)

// Make sure the type for shader_inputs can hold all shader inputs
typedef uint8_t shader_inputs_t;
static_assert((1llu << (8 * sizeof(shader_inputs_t))) > SHADER_INPUT_BITS, "Type for shader_inputs is not large enough to hold all shader inputs (SHADER_INPUT_BITS)");

#pragma pack(1)
struct ColorCombiner {
    uint8_t ccs[CC_CYCLES][2][CC_COMPONENTS];
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
            uint8_t use_noise            : 1;
            uint8_t use_ext_color        : 1;
            uint8_t use_ext_alpha        : 1;
            uint8_t ext_color_alpha_same : 1;
            uint8_t used_textures        : MAX_TEXTURES;
        };
        uint8_t features;
    };
    const struct ExtendedCombiner *ec[2];
};
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

void gfx_cc_generate(struct ColorCombiner *cc, uint8_t cycle, bool alpha, uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void gfx_cc_load_extended_combiner(struct ColorCombiner *cc, bool alpha, const char *str);
void gfx_cc_update_features(struct ColorCombiner *cc);

#ifdef __cplusplus
}
#endif

#endif
