#include "PR/gbi.h"
#include "gfx_cc.h"
#include "gfx_pc.h"

// From https://ultra64.ca/files/documentation/online-manuals/man/pro-man/pro12/index12.6.html
// "Extended" entries are coopdx specific, allowing for more combiners with existing sources
// Table indices are, in this order: RGB/A, ABCD, G_MUX, cycle
static const uint8_t CC_COMPONENTS_TO_CCS[2][CC_COMPONENTS][32][CC_CYCLES] = {

/////////
// RGB //
/////////
{

// A
{
[G_CCMUX_COMBINED]        = { CCS_0,       CCS_COMBINED  },
[G_CCMUX_TEXEL0]          = { CCS_TEXEL0,  CCS_TEXEL1    },
[G_CCMUX_TEXEL1]          = { CCS_TEXEL1,  CCS_TEXEL0    },
[G_CCMUX_PRIMITIVE]       = { CCS_PRIM,    CCS_PRIM      },
[G_CCMUX_SHADE]           = { CCS_SHADE,   CCS_SHADE     },
[G_CCMUX_ENVIRONMENT]     = { CCS_ENV,     CCS_ENV       },
[G_CCMUX_1]               = { CCS_1,       CCS_1         },
[G_CCMUX_0]               = { CCS_0,       CCS_0         },
[G_CCMUX_NOISE]           = { CCS_NOISE,   CCS_COMBINEDA },
// Extended
[G_CCMUX_TEXEL0_ALPHA]    = { CCS_TEXEL0A, CCS_TEXEL1A   },
[G_CCMUX_TEXEL1_ALPHA]    = { CCS_TEXEL1A, CCS_TEXEL0A   },
[G_CCMUX_PRIMITIVE_ALPHA] = { CCS_PRIMA,   CCS_PRIMA     },
[G_CCMUX_SHADE_ALPHA]     = { CCS_SHADEA,  CCS_SHADEA    },
[G_CCMUX_ENV_ALPHA]       = { CCS_ENVA,    CCS_ENVA      },
},

// B
{
[G_CCMUX_COMBINED]        = { CCS_0,       CCS_COMBINED  },
[G_CCMUX_TEXEL0]          = { CCS_TEXEL0,  CCS_TEXEL1    },
[G_CCMUX_TEXEL1]          = { CCS_TEXEL1,  CCS_TEXEL0    },
[G_CCMUX_PRIMITIVE]       = { CCS_PRIM,    CCS_PRIM      },
[G_CCMUX_SHADE]           = { CCS_SHADE,   CCS_SHADE     },
[G_CCMUX_ENVIRONMENT]     = { CCS_ENV,     CCS_ENV       },
// [G_CCMUX_CENTER]       = { CCS_CENTER,  CCS_CENTER    },
// [G_CCMUX_K4]           = { CCS_K4,      CCS_K4        },
[G_CCMUX_0]               = { CCS_0,       CCS_0         },
// Extended
[G_CCMUX_COMBINED_ALPHA]  = { CCS_0,       CCS_COMBINEDA },
[G_CCMUX_TEXEL0_ALPHA]    = { CCS_TEXEL0A, CCS_TEXEL1A   },
[G_CCMUX_TEXEL1_ALPHA]    = { CCS_TEXEL1A, CCS_TEXEL0A   },
[G_CCMUX_PRIMITIVE_ALPHA] = { CCS_PRIMA,   CCS_PRIMA     },
[G_CCMUX_SHADE_ALPHA]     = { CCS_SHADEA,  CCS_SHADEA    },
[G_CCMUX_ENV_ALPHA]       = { CCS_ENVA,    CCS_ENVA      },
},

// C
{
[G_CCMUX_COMBINED]        = { CCS_0,                 CCS_COMBINED          },
[G_CCMUX_TEXEL0]          = { CCS_TEXEL0,            CCS_TEXEL1            },
[G_CCMUX_TEXEL1]          = { CCS_TEXEL1,            CCS_TEXEL0            },
[G_CCMUX_PRIMITIVE]       = { CCS_PRIM,              CCS_PRIM              },
[G_CCMUX_SHADE]           = { CCS_SHADE,             CCS_SHADE             },
[G_CCMUX_ENVIRONMENT]     = { CCS_ENV,               CCS_ENV               },
// [G_CCMUX_CENTER]       = { CCS_CENTER,            CCS_CENTER            },
[G_CCMUX_COMBINED_ALPHA]  = { CCS_0,                 CCS_COMBINEDA         },
[G_CCMUX_TEXEL0_ALPHA]    = { CCS_TEXEL0A,           CCS_TEXEL0A           },
[G_CCMUX_TEXEL1_ALPHA]    = { CCS_TEXEL1A,           CCS_TEXEL1A           },
[G_CCMUX_PRIMITIVE_ALPHA] = { CCS_PRIMA,             CCS_PRIMA             },
[G_CCMUX_SHADE_ALPHA]     = { CCS_SHADEA,            CCS_SHADEA            },
[G_CCMUX_ENV_ALPHA]       = { CCS_ENVA,              CCS_ENVA              },
[G_CCMUX_LOD_FRACTION]    = { CCS_LOD,               CCS_LOD               },
// [G_CCMUX_PRIM_LOD_FRAC]= { CCS_PRIM_LOD_FRACTION, CCS_PRIM_LOD_FRACTION },
// [G_CCMUX_K5]           = { CCS_K5,                CCS_K5                },
[G_CCMUX_0]               = { CCS_0,                 CCS_0                 },
},

// D
{
[G_CCMUX_COMBINED]        = { CCS_0,       CCS_COMBINED },
[G_CCMUX_TEXEL0]          = { CCS_TEXEL0,  CCS_TEXEL1   },
[G_CCMUX_TEXEL1]          = { CCS_TEXEL1,  CCS_TEXEL0   },
[G_CCMUX_PRIMITIVE]       = { CCS_PRIM,    CCS_PRIM     },
[G_CCMUX_SHADE]           = { CCS_SHADE,   CCS_SHADE    },
[G_CCMUX_ENVIRONMENT]     = { CCS_ENV,     CCS_ENV      },
[G_CCMUX_1]               = { CCS_1,       CCS_1        },
[G_CCMUX_0]               = { CCS_0,       CCS_0        },
[G_CCMUX_TEXEL0_ALPHA]    = { CCS_TEXEL0A, CCS_TEXEL1A  },
[G_CCMUX_TEXEL1_ALPHA]    = { CCS_TEXEL1A, CCS_TEXEL0A  },
[G_CCMUX_PRIMITIVE_ALPHA] = { CCS_PRIMA,   CCS_PRIMA    },
[G_CCMUX_SHADE_ALPHA]     = { CCS_SHADEA,  CCS_SHADEA   },
[G_CCMUX_ENV_ALPHA]       = { CCS_ENVA,    CCS_ENVA     },
},

},

///////////
// ALPHA //
///////////
{

// A
{
[G_ACMUX_COMBINED]    = { CCS_0,       CCS_COMBINEDA },
[G_ACMUX_TEXEL0]      = { CCS_TEXEL0A, CCS_TEXEL1A   },
[G_ACMUX_TEXEL1]      = { CCS_TEXEL1A, CCS_TEXEL0A   },
[G_ACMUX_PRIMITIVE]   = { CCS_PRIMA,   CCS_PRIMA     },
[G_ACMUX_SHADE]       = { CCS_SHADEA,  CCS_SHADEA    },
[G_ACMUX_ENVIRONMENT] = { CCS_ENVA,    CCS_ENVA      },
[G_ACMUX_1]           = { CCS_1,       CCS_1         },
[G_ACMUX_0]           = { CCS_0,       CCS_0         },
},

// B
{
[G_ACMUX_COMBINED]    = { CCS_0,       CCS_COMBINEDA },
[G_ACMUX_TEXEL0]      = { CCS_TEXEL0A, CCS_TEXEL1A   },
[G_ACMUX_TEXEL1]      = { CCS_TEXEL1A, CCS_TEXEL0A   },
[G_ACMUX_PRIMITIVE]   = { CCS_PRIMA,   CCS_PRIMA     },
[G_ACMUX_SHADE]       = { CCS_SHADEA,  CCS_SHADEA    },
[G_ACMUX_ENVIRONMENT] = { CCS_ENVA,    CCS_ENVA      },
[G_ACMUX_1]           = { CCS_1,       CCS_1         },
[G_ACMUX_0]           = { CCS_0,       CCS_0         },
},

// C
{
[G_ACMUX_LOD_FRACTION]  = { CCS_LOD,               CCS_LOD               },
[G_ACMUX_TEXEL0]        = { CCS_TEXEL0A,           CCS_TEXEL1A           },
[G_ACMUX_TEXEL1]        = { CCS_TEXEL1A,           CCS_TEXEL0A           },
[G_ACMUX_PRIMITIVE]     = { CCS_PRIMA,             CCS_PRIMA             },
[G_ACMUX_SHADE]         = { CCS_SHADEA,            CCS_SHADEA            },
[G_ACMUX_ENVIRONMENT]   = { CCS_ENVA,              CCS_ENVA              },
// [G_CCMUX_PRIM_LOD_FRAC] = { CCS_PRIM_LOD_FRACTION, CCS_PRIM_LOD_FRACTION },
[G_ACMUX_0]             = { CCS_0,                 CCS_0                 },
},

// D
{
[G_ACMUX_COMBINED]    = { CCS_0,       CCS_COMBINEDA },
[G_ACMUX_TEXEL0]      = { CCS_TEXEL0A, CCS_TEXEL1A   },
[G_ACMUX_TEXEL1]      = { CCS_TEXEL1A, CCS_TEXEL0A   },
[G_ACMUX_PRIMITIVE]   = { CCS_PRIMA,   CCS_PRIMA     },
[G_ACMUX_SHADE]       = { CCS_SHADEA,  CCS_SHADEA    },
[G_ACMUX_ENVIRONMENT] = { CCS_ENVA,    CCS_ENVA      },
[G_ACMUX_1]           = { CCS_1,       CCS_1         },
[G_ACMUX_0]           = { CCS_0,       CCS_0         },
},

}
};

static inline void color_comb(uint8_t *ccs, uint8_t cycle, bool alpha, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    ccs[0] = (a < 32) ? CC_COMPONENTS_TO_CCS[alpha][0][a][cycle] : CCS_0;
    ccs[1] = (b < 32) ? CC_COMPONENTS_TO_CCS[alpha][1][b][cycle] : CCS_0;
    ccs[2] = (c < 32) ? CC_COMPONENTS_TO_CCS[alpha][2][c][cycle] : CCS_0;
    ccs[3] = (d < 32) ? CC_COMPONENTS_TO_CCS[alpha][3][d][cycle] : CCS_0;
}

void gfx_cc_generate_from_combine_mode(struct ColorCombiner *cc,
    uint8_t ccmux_a0, uint8_t ccmux_b0, uint8_t ccmux_c0, uint8_t ccmux_d0,
    uint8_t acmux_a0, uint8_t acmux_b0, uint8_t acmux_c0, uint8_t acmux_d0,
    uint8_t ccmux_a1, uint8_t ccmux_b1, uint8_t ccmux_c1, uint8_t ccmux_d1,
    uint8_t acmux_a1, uint8_t acmux_b1, uint8_t acmux_c1, uint8_t acmux_d1
) {
    color_comb(cc->ccs[0][0], 0, 0, ccmux_a0, ccmux_b0, ccmux_c0, ccmux_d0);
    color_comb(cc->ccs[0][1], 0, 1, acmux_a0, acmux_b0, acmux_c0, acmux_d0);
    color_comb(cc->ccs[1][0], 1, 0, ccmux_a1, ccmux_b1, ccmux_c1, ccmux_d1);
    color_comb(cc->ccs[1][1], 1, 1, acmux_a1, acmux_b1, acmux_c1, acmux_d1);
    cc->ec[0] = NULL;
    cc->ec[1] = NULL;
}

void gfx_cc_generate_from_sources(struct ColorCombiner *cc, bool alpha,
    uint8_t ccs_a0, uint8_t ccs_b0, uint8_t ccs_c0, uint8_t ccs_d0,
    uint8_t ccs_a1, uint8_t ccs_b1, uint8_t ccs_c1, uint8_t ccs_d1
) {
    cc->ccs[0][alpha][0] = ccs_a0;
    cc->ccs[0][alpha][1] = ccs_b0;
    cc->ccs[0][alpha][2] = ccs_c0;
    cc->ccs[0][alpha][3] = ccs_d0;
    cc->ccs[1][alpha][0] = ccs_a1;
    cc->ccs[1][alpha][1] = ccs_b1;
    cc->ccs[1][alpha][2] = ccs_c1;
    cc->ccs[1][alpha][3] = ccs_d1;
    cc->ec[alpha] = NULL;
}

void gfx_cc_load_extended_combiner(struct ColorCombiner *cc, bool alpha, const char *str) {
    cc->ec[alpha] = gfx_cc_ext_get_combiner(str, NULL);
}

void gfx_cc_update_features(struct ColorCombiner *cc) {
    cc->shader_inputs = 0;
    cc->features = 0;

    // Enable extended combiners
    cc->use_ext_color = cc->ec[0] != NULL;
    cc->use_ext_alpha = cc->ec[1] != NULL;
    cc->ext_color_alpha_same = (cc->use_ext_color && cc->use_ext_alpha && memcmp(cc->ec[0], cc->ec[1], sizeof(struct ExtendedCombiner)) == 0);
    for (int i = 0; i < (cc->ext_color_alpha_same ? 1 : 2); ++i) {
        const struct ExtendedCombiner *ec = cc->ec[i];
        if (ec != NULL) {
            for (uint32_t j = 0; j < ec->num_ops; ++j) {
                uint8_t op = ec->ops[j];
                switch (op) {
                    case CC_EXT_NOISE:
                        cc->use_noise = TRUE;
                        break;
                    case CC_EXT_PRIM:
                    case CC_EXT_PRIMA:
                        cc->shader_inputs |= SHADER_INPUT_PRIMITIVE;
                        break;
                    case CC_EXT_SHADE:
                    case CC_EXT_SHADEA:
                        cc->shader_inputs |= SHADER_INPUT_SHADE;
                        break;
                    case CC_EXT_ENV:
                    case CC_EXT_ENVA:
                        cc->shader_inputs |= SHADER_INPUT_ENVIRONMENT;
                        break;
                    case CC_EXT_LOD:
                        cc->shader_inputs |= SHADER_INPUT_LOD;
                        break;
                    case CC_EXT_AMBIENT:
                        cc->shader_inputs |= SHADER_INPUT_AMBIENT;
                        break;
                    default:
                        // if (op >= CC_EXT_LIGHT0 && op < CC_EXT_TEXEL0) {
                        //     cc->shader_inputs |= SHADER_INPUT_LIGHTS;
                        //     break;
                        // }
                        if (op >= CC_EXT_TEXEL0 && op < CC_EXT_VALUE_COUNT) {
                            cc->used_textures |= USED_TEXTURE((op - CC_EXT_TEXEL0) / 2);
                            break;
                        }
                        break;
                }
            }
        }
    }

    // Determine shader inputs and features
    for (int cycle = 0; cycle < (cc->use_2cycle ? 2 : 1); ++cycle)
    for (int i = 0; i < 2; ++i) {
        if (cc->ec[i] == NULL) {
            for (int j = 0; j < CC_COMPONENTS; ++j) {
                uint8_t ccs = cc->ccs[cycle][i][j];
                switch (ccs) {
                    case CCS_TEXEL0:
                    case CCS_TEXEL0A:
                        cc->used_textures |= USED_TEXTURE(0);
                        break;
                    case CCS_TEXEL1:
                    case CCS_TEXEL1A:
                        cc->used_textures |= USED_TEXTURE(1);
                        break;
                    case CCS_NOISE:
                        cc->use_noise = TRUE;
                        break;
                    case CCS_PRIM:
                    case CCS_PRIMA:
                        cc->shader_inputs |= SHADER_INPUT_PRIMITIVE;
                        break;
                    case CCS_SHADE:
                    case CCS_SHADEA:
                        cc->shader_inputs |= SHADER_INPUT_SHADE;
                        break;
                    case CCS_ENV:
                    case CCS_ENVA:
                        cc->shader_inputs |= SHADER_INPUT_ENVIRONMENT;
                        break;
                    case CCS_LOD:
                        cc->shader_inputs |= SHADER_INPUT_LOD;
                        break;
                }
            }
        }
    }
}
