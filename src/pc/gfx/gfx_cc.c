#include <stdio.h>
#include "PR/gbi.h"
#include "gfx_cc.h"
#include "gfx_pc.h"

static u8 sAllowCCPrint = 1;

bool gfx_cm_uses_second_texture(struct CombineMode* cm) {
    for (int i = 0; i < 16; i++) {
        u8 v = cm->all_values[i];
        switch (v) {
            case CC_TEXEL1:
            case CC_TEXEL1A:
                return true;
        }
    }
    return false;
}

void gfx_cc_get_features(struct ColorCombiner* cc, struct CCFeatures* ccf) {
    // reset ccf
    memset(ccf, 0, sizeof(struct CCFeatures));

    int cmd_length = cc->cm.use_2cycle ? 16 : 8;
    for (int i = 0; i < cmd_length; i++) {
        u8 c = cc->shader_commands[i];
        if (c >= SHADER_INPUT_1 && c < SHADER_INPUT_1 + SHADER_MAX_INPUTS) {
            if (c > ccf->num_inputs) { ccf->num_inputs = c; }
        }
        ccf->used_textures[0] = ccf->used_textures[0] || c == SHADER_TEXEL0 || c == SHADER_TEXEL0A;
        ccf->used_textures[1] = ccf->used_textures[1] || c == SHADER_TEXEL1 || c == SHADER_TEXEL1A;
        ccf->do_noise = ccf->do_noise || c == SHADER_NOISE;
    }

    // figure out optimizations
    for (int i = 0; i < 16 / 4; i++) {
        u8* c = &cc->shader_commands[i * 4];
        ccf->do_single[i]   = (c[2] == 0);
        ccf->do_multiply[i] = (c[1] == 0 && c[3] == 0);
        ccf->do_mix[i]      = (c[1] == c[3]);
    }

    ccf->color_alpha_same[0] = 1;
    ccf->color_alpha_same[1] = 1;

    for (int i = 0; i < 2; i++) {
        u8* cmd = &cc->shader_commands[i * 8];
        for (int j = 0; j < 4; j++) {
            if (cmd[j] != cmd[j + 4]) {
                ccf->color_alpha_same[i] = 0;
                break;
            }
        }
    }
}

void gfx_cc_print(UNUSED struct ColorCombiner *cc) {
    if (!sAllowCCPrint) { return; }
    // disabled for now
    return;

/*#ifdef DEVELOPMENT
    if (!sAllowCCPrint) { return; }

    printf("\n>> gfx_pc_precomp_shader(");

    struct CombineMode* cm = &cc->cm;
    printf("0x%08x, ", cm->rgb1);
    printf("0x%08x, ", cm->alpha1);
    printf("0x%08x, ", cm->rgb2);
    printf("0x%08x, ", cm->alpha2);
    printf("0x%08x",   cm->flags);

    printf(");");
    printf("    // %016" PRIx64, cm->hash);
    printf("\n");
#endif*/
}

static uint8_t color_comb_component_a(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_CCMUX_COMBINED:        return cycle ? CC_COMBINED : CC_0;
        case G_CCMUX_TEXEL0:          return cycle ? CC_TEXEL1 : CC_TEXEL0;
        case G_CCMUX_TEXEL1:          return cycle ? CC_TEXEL0 : CC_TEXEL1;
        case G_CCMUX_PRIMITIVE:       return CC_PRIM;
        case G_CCMUX_SHADE:           return CC_SHADE;
        case G_CCMUX_ENVIRONMENT:     return CC_ENV;
        case G_CCMUX_1:               return CC_1;
        case G_CCMUX_0:               return CC_0;

        case G_CCMUX_COMBINED_ALPHA:  return cycle ? CC_COMBINEDA : CC_NOISE;
        case G_CCMUX_TEXEL0_ALPHA:    return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_CCMUX_TEXEL1_ALPHA:    return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_CCMUX_PRIMITIVE_ALPHA: return CC_PRIMA;
        case G_CCMUX_SHADE_ALPHA:     return CC_SHADEA;
        case G_CCMUX_ENV_ALPHA:       return CC_ENVA;

        default:                      return CC_0;
    }
}

static uint8_t color_comb_component_b(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_CCMUX_COMBINED:          return cycle ? CC_COMBINED : CC_0;
        case G_CCMUX_TEXEL0:            return cycle ? CC_TEXEL1 : CC_TEXEL0;
        case G_CCMUX_TEXEL1:            return cycle ? CC_TEXEL0 : CC_TEXEL1;
        case G_CCMUX_PRIMITIVE:         return CC_PRIM;
        case G_CCMUX_SHADE:             return CC_SHADE;
        case G_CCMUX_ENVIRONMENT:       return CC_ENV;
        //case G_CCMUX_CENTER:          return CC_CENTER; // is this correct for "Chrome Key Center"?
        //case G_CCMUX_K4:              return CC_K4;
        case G_CCMUX_0:                 return CC_0;

        case G_CCMUX_COMBINED_ALPHA:    return cycle ? CC_COMBINEDA : CC_0;
        case G_CCMUX_TEXEL0_ALPHA:      return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_CCMUX_TEXEL1_ALPHA:      return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_CCMUX_PRIMITIVE_ALPHA:   return CC_PRIMA;
        case G_CCMUX_SHADE_ALPHA:       return CC_SHADEA;
        case G_CCMUX_ENV_ALPHA:         return CC_ENVA;

        default:                        return CC_0;
    }
}

static uint8_t color_comb_component_c(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_CCMUX_COMBINED:          return cycle ? CC_COMBINED : CC_0;
        case G_CCMUX_TEXEL0:            return cycle ? CC_TEXEL1 : CC_TEXEL0;
        case G_CCMUX_TEXEL1:            return cycle ? CC_TEXEL0 : CC_TEXEL1;
        case G_CCMUX_PRIMITIVE:         return CC_PRIM;
        case G_CCMUX_SHADE:             return CC_SHADE;
        case G_CCMUX_ENVIRONMENT:       return CC_ENV;
        //case G_CCMUX_CENTER:          return CC_CENTER; // is this correct for "Chrome Key Center"?
        case G_CCMUX_COMBINED_ALPHA:    return cycle ? CC_COMBINEDA : CC_0;
        case G_CCMUX_TEXEL0_ALPHA:      return CC_TEXEL0A;
        case G_CCMUX_TEXEL1_ALPHA:      return CC_TEXEL1A;
        case G_CCMUX_PRIMITIVE_ALPHA:   return CC_PRIMA;
        case G_CCMUX_SHADE_ALPHA:       return CC_SHADEA;
        case G_CCMUX_ENV_ALPHA:         return CC_ENVA;
        case G_CCMUX_LOD_FRACTION:      return CC_LOD;
        //case G_CCMUX_PRIM_LOD_FRAC:   return CC_PRIM_LOD_FRACTION;
        //case G_CCMUX_K5:              return CC_K5;
        case G_CCMUX_0:                 return CC_0;
        default:                        return CC_0;
    }
}

static uint8_t color_comb_component_d(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_CCMUX_COMBINED:          return cycle ? CC_COMBINED : CC_0;
        case G_CCMUX_TEXEL0:            return cycle ? CC_TEXEL1 : CC_TEXEL0;
        case G_CCMUX_TEXEL1:            return cycle ? CC_TEXEL0 : CC_TEXEL1;
        case G_CCMUX_PRIMITIVE:         return CC_PRIM;
        case G_CCMUX_SHADE:             return CC_SHADE;
        case G_CCMUX_ENVIRONMENT:       return CC_ENV;
        case G_CCMUX_1:                 return CC_1;
        case G_CCMUX_0:                 return CC_0;

        case G_CCMUX_TEXEL0_ALPHA:      return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_CCMUX_TEXEL1_ALPHA:      return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_CCMUX_PRIMITIVE_ALPHA:   return CC_PRIMA;
        case G_CCMUX_SHADE_ALPHA:       return CC_SHADEA;
        case G_CCMUX_ENV_ALPHA:         return CC_ENVA;

        default:                  return CC_0;
    }
}

uint32_t color_comb_rgb(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t cycle) {
    return color_comb_component_a(a, cycle)
        | (color_comb_component_b(b, cycle) << 8)
        | (color_comb_component_c(c, cycle) << 16)
        | (color_comb_component_d(d, cycle) << 24);
}

static uint8_t color_comb_component_a_alpha(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_ACMUX_COMBINED:          return cycle ? CC_COMBINEDA : CC_0;
        case G_ACMUX_TEXEL0:            return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_ACMUX_TEXEL1:            return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_ACMUX_PRIMITIVE:         return CC_PRIMA;
        case G_ACMUX_SHADE:             return CC_SHADEA;
        case G_ACMUX_ENVIRONMENT:       return CC_ENVA;
        case G_ACMUX_1:                 return CC_1;
        case G_ACMUX_0:                 return CC_0;
        default:                        return CC_0;
    }
}

static uint8_t color_comb_component_b_alpha(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_ACMUX_COMBINED:          return cycle ? CC_COMBINEDA : CC_0;
        case G_ACMUX_TEXEL0:            return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_ACMUX_TEXEL1:            return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_ACMUX_PRIMITIVE:         return CC_PRIMA;
        case G_ACMUX_SHADE:             return CC_SHADEA;
        case G_ACMUX_ENVIRONMENT:       return CC_ENVA;
        case G_ACMUX_1:                 return CC_1;
        case G_ACMUX_0:                 return CC_0;
        default:                        return CC_0;
    }
}

static uint8_t color_comb_component_c_alpha(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_ACMUX_LOD_FRACTION:      return CC_LOD;
        case G_ACMUX_TEXEL0:            return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_ACMUX_TEXEL1:            return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_ACMUX_PRIMITIVE:         return CC_PRIMA;
        case G_ACMUX_SHADE:             return CC_SHADEA;
        case G_ACMUX_ENVIRONMENT:       return CC_ENVA;
        //case G_CCMUX_PRIM_LOD_FRAC:   return CC_PRIM_LOD_FRACTION;
        case G_ACMUX_0:                 return CC_0;
        default:                        return CC_0;
    }
}

static uint8_t color_comb_component_d_alpha(uint32_t v, uint8_t cycle) {
    switch (v) {
        case G_ACMUX_COMBINED:          return cycle ? CC_COMBINEDA : CC_0;
        case G_ACMUX_TEXEL0:            return cycle ? CC_TEXEL1A : CC_TEXEL0A;
        case G_ACMUX_TEXEL1:            return cycle ? CC_TEXEL0A : CC_TEXEL1A;
        case G_ACMUX_PRIMITIVE:         return CC_PRIMA;
        case G_ACMUX_SHADE:             return CC_SHADEA;
        case G_ACMUX_ENVIRONMENT:       return CC_ENVA;
        case G_ACMUX_1:                 return CC_1;
        case G_ACMUX_0:                 return CC_0;
        default:                        return CC_0;
    }
}

uint32_t color_comb_alpha(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint8_t cycle) {
    return color_comb_component_a_alpha(a, cycle)
        | (color_comb_component_b_alpha(b, cycle) << 8)
        | (color_comb_component_c_alpha(c, cycle) << 16)
        | (color_comb_component_d_alpha(d, cycle) << 24);
}
