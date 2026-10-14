#include <stdio.h>
#include "PR/gbi.h"
#include "gfx_cc.h"
#include "gfx_pc.h"

static bool sAllowCCPrint = true;

bool gfx_cm_uses_second_texture(struct CombineMode *cm) {
    for (int i = 0; i < ARRAY_COUNT(cm->all_values); i++) {
        switch (cm->all_values[i]) {
            case CC_TEXEL1:
            case CC_TEXEL1A:
                return true;
        }
    }
    return false;
}

void gfx_cc_generate_shader_commands(struct ColorCombiner *cc) {
    uint8_t next_input_number = 0;
    uint8_t input_number[CC_ENUM_MAX] = { 0 };

    for (int i = 0; i < CC_CMD_LENGTH; i++) {
        uint8_t cm_cmd = cc->cm.all_values[i];
        uint8_t shader_cmd = 0;
        switch (cm_cmd) {
            case CC_0:
                shader_cmd = SHADER_0;
                break;
            case CC_1:
                shader_cmd = SHADER_1;
                break;
            case CC_TEXEL0:
                shader_cmd = SHADER_TEXEL0;
                break;
            case CC_TEXEL1:
                shader_cmd = SHADER_TEXEL1;
                break;
            case CC_TEXEL0A:
                shader_cmd = SHADER_TEXEL0A;
                break;
            case CC_TEXEL1A:
                shader_cmd = SHADER_TEXEL1A;
                break;
            case CC_COMBINED:
                shader_cmd = cc->cm.use_2cycle ? SHADER_COMBINED : SHADER_0;
                break;
            case CC_COMBINEDA:
                shader_cmd = cc->cm.use_2cycle ? SHADER_COMBINEDA : SHADER_0;
                break;
            case CC_NOISE:
                shader_cmd = SHADER_NOISE;
                break;
            case CC_PRIM:
            case CC_PRIMA:
            case CC_SHADE:
            case CC_SHADEA:
            case CC_ENV:
            case CC_ENVA:
            case CC_LOD:
                if (input_number[cm_cmd] == 0) {
                    cc->shader_input_mapping[next_input_number] = cm_cmd;
                    input_number[cm_cmd] = SHADER_INPUTS + next_input_number;
                    next_input_number++;
                }
                shader_cmd = input_number[cm_cmd];
                break;
            default:
                shader_cmd = SHADER_0;
                break;
        }
        cc->shader_commands[i] = shader_cmd;
    }

    gfx_cc_print(cc);
}

void gfx_cc_get_features(struct ColorCombiner *cc, struct CCFeatures *ccf) {
    // reset ccf
    memset(ccf, 0, sizeof(struct CCFeatures));

    // determine features
    int cmd_length = (CC_COMPONENTS * CC_CHANNELS) * (cc->cm.use_2cycle ? 2 : 1);
    for (int i = 0; i < cmd_length; i++) {
        uint8_t cmd = cc->shader_commands[i];
        if (cmd >= SHADER_INPUTS && cmd < SHADER_ENUM_MAX) {
            ccf->num_inputs = MAX(ccf->num_inputs, (cmd - SHADER_INPUTS + 1));
        }
        if (cmd == SHADER_TEXEL0 || cmd == SHADER_TEXEL0A) {
            ccf->used_textures[0] = true;
        }
        if (cmd == SHADER_TEXEL1 || cmd == SHADER_TEXEL1A) {
            ccf->used_textures[1] = true;
        }
        if (cmd == SHADER_NOISE) {
            ccf->do_noise = true;
        }
    }

    // figure out optimizations
    for (int i = 0; i < CC_MODES; i++) {
        const uint8_t *abcd = &cc->shader_commands[i * CC_COMPONENTS];
        ccf->do_single[i]   = (abcd[2] == SHADER_0);                         // D
        ccf->do_multiply[i] = (abcd[1] == SHADER_0 && abcd[3] == SHADER_0);  // A * C
        ccf->do_mix[i]      = (abcd[1] == abcd[3]);                          // (A - B) * C + B = lerp(C, B, A)
    }

    for (int i = 0; i < CC_CYCLES; i++) {
        ccf->color_alpha_same[i] = true;
        uint8_t *cmd = &cc->shader_commands[i * (CC_COMPONENTS * CC_CHANNELS)];
        for (int j = 0; j < CC_COMPONENTS; j++) { // TODO: memcmp?
            if (cmd[j] != cmd[j + CC_COMPONENTS]) {
                ccf->color_alpha_same[i] = false;
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


// TODO: table this shit

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

        default:                        return CC_0;
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
