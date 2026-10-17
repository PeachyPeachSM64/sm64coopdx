#include <stdio.h>
#include "PR/gbi.h"
#include "gfx_cc.h"
#include "gfx_pc.h"
#include "platform_info.h"

bool gfx_cm_uses_second_texture(struct CombineMode *cm) {
    for (int i = 0; i < ARRAY_COUNT(cm->all_values); i++) {
        switch (cm->all_values[i]) {
            case CCS_TEXEL1:
            case CCS_TEXEL1A:
                return true;
        }
    }
    return false;
}

bool gfx_cc_generate(struct ColorCombiner *cc, struct CombineMode *cm) {
    if (!cc || !cm) {
        return false;
    }
    memset(cc, 0, sizeof(*cc));

    // Set combine mode
    cc->cm = *cm;

    // Determine shader inputs
    int num_ccs = (ARRAY_COUNT(cc->cm.all_values) / CC_CYCLES) * (cc->cm.use_2cycle ? 2 : 1);
    for (int i = 0; i < num_ccs; ++i) {
        uint8_t ccs = cc->cm.all_values[i];
        if (ccs >= SHADER_INPUT_FIRST && ccs <= SHADER_INPUT_LAST) {
            cc->shader_inputs |= (1 << (ccs - SHADER_INPUT_FIRST));
        }
    }

    // Determine features
    for (int i = 0; i < num_ccs; ++i) {
        uint8_t ccs = cc->cm.all_values[i];
        switch (ccs) {
            case CCS_TEXEL0:
            case CCS_TEXEL0A:
                cc->used_textures[0] = true;
                break;
            case CCS_TEXEL1:
            case CCS_TEXEL1A:
                cc->used_textures[1] = true;
                break;
            case CCS_NOISE:
                cc->do_noise = true;
                break;
        }
    }

    // Figure out source optimizations
    int num_modes = CC_CHANNELS * (cc->cm.use_2cycle ? 2 : 1);
    for (int i = 0; i < num_modes; ++i) {
        const uint8_t *ABCD = &cc->cm.all_values[i * CC_COMPONENTS];
        cc->do_single[i]   = (ABCD[2] == CCS_0);                      // D
        cc->do_multiply[i] = (ABCD[1] == CCS_0 && ABCD[3] == CCS_0);  // A * C
        cc->do_mix[i]      = (ABCD[1] == ABCD[3]);                    // (A - B) * C + B = lerp(C, B, A)
    }

    int num_cycles = cc->cm.use_2cycle ? 2 : 1;
    for (int i = 0; i < num_cycles; i++) {
        cc->color_alpha_same[i] = cc->cm.cycles[i].rgb == cc->cm.cycles[i].alpha;
    }

    return true;
}

// From https://ultra64.ca/files/documentation/online-manuals/man/pro-man/pro12/index12.6.html
// "Extended" entries are coopdx specific, allowing for more combiners with existing sources
// Table indices are, in this order: RGB/A, ABCD, G_MUX, cycle
static const uint8_t CC_COMPONENTS_TO_CCS[CC_CHANNELS][CC_COMPONENTS][32][CC_CYCLES] = {

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
[G_CCMUX_COMBINED_ALPHA]  = { CCS_NOISE,   CCS_COMBINEDA },
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

uint32_t color_comb(bool alpha, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t cycle) {
    uint8_t A = (a < 32) ? CC_COMPONENTS_TO_CCS[alpha][0][a][cycle] : CCS_0;
    uint8_t B = (b < 32) ? CC_COMPONENTS_TO_CCS[alpha][1][b][cycle] : CCS_0;
    uint8_t C = (c < 32) ? CC_COMPONENTS_TO_CCS[alpha][2][c][cycle] : CCS_0;
    uint8_t D = (d < 32) ? CC_COMPONENTS_TO_CCS[alpha][3][d][cycle] : CCS_0;
#if IS_BIG_ENDIAN
    return (A << 24) | (B << 16) | (C << 8) | (D << 0);
#else
    return (A << 0) | (B << 8) | (C << 16) | (D << 24);
#endif
}
