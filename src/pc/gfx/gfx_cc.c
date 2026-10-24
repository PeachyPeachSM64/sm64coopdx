#include <stdio.h>
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

void gfx_cc_generate(struct ColorCombiner *cc,
    uint8_t  a0, uint8_t  b0, uint8_t  c0, uint8_t  d0,
    uint8_t Aa0, uint8_t Ab0, uint8_t Ac0, uint8_t Ad0,
    uint8_t  a1, uint8_t  b1, uint8_t  c1, uint8_t  d1,
    uint8_t Aa1, uint8_t Ab1, uint8_t Ac1, uint8_t Ad1
) {
    memset(cc, 0, sizeof(*cc));
    color_comb(cc->ccs[0][0], 0, 0, a0, b0, c0, d0);
    color_comb(cc->ccs[0][1], 0, 1, Aa0, Ab0, Ac0, Ad0);
    color_comb(cc->ccs[1][0], 1, 0, a1, b1, c1, d1);
    color_comb(cc->ccs[1][1], 1, 1, Aa1, Ab1, Ac1, Ad1);
}

void gfx_cc_update_features(struct ColorCombiner *cc) {
    cc->shader_inputs = 0;
    cc->features = 0;

    // Determine shader inputs and features
    for (int cycle = 0; cycle < (cc->use_2cycle ? 2 : 1); ++cycle)
    for (int i = 0; i < 2; ++i)
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
