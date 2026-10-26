#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "PR/gbi.h"
#include "gfx_cc.h"
#include "gfx_pc.h"
#include "game/memory.h"

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



//
// Extended color combiners
//

// static const char *CCS_NAMES[][2] = {
//     [CCS_TEXEL0]  = { "TEXEL0",  "TEXEL0"          },
//     [CCS_TEXEL0A] = { "TEXEL0A", "TEXEL0_ALPHA"    },
//     [CCS_TEXEL1]  = { "TEXEL1",  "TEXEL1"          },
//     [CCS_TEXEL1A] = { "TEXEL1A", "TEXEL1_ALPHA"    },
//     [CCS_NOISE]   = { "NOISE",   "NOISE"           },
//     [CCS_PRIM]    = { "PRIM",    "PRIMITIVE"       },
//     [CCS_PRIMA]   = { "PRIMA",   "PRIMITIVE_ALPHA" },
//     [CCS_SHADE]   = { "SHADE",   "SHADE"           },
//     [CCS_SHADEA]  = { "SHADEA",  "SHADE_ALPHA"     },
//     [CCS_ENV]     = { "ENV",     "ENVIRONMENT"     },
//     [CCS_ENVA]    = { "ENVA",    "ENV_ALPHA"       },
//     [CCS_LOD]     = { "LOD",     "LOD_FRACTION"    },
// };

// TEXEL0A      -> TEXEL0.aaaa

// vec(r, g, b, a) = r.r, g.g, b.b, a.a
// vec(TEXEL0, rot(TEXEL0, 3), rot(TEXEL0, 2), rot(TEXEL0, 1)) = TEXEL0.rrrr
// vec(TEXEL0, 0, 0, 1)

// operator .
// TEXEL0.r -> [OP_DOT] [VAL_TEXEL0] [rrggbbaa]
// right operand of . must be "x", "xxx" or "xxxx", where x must be one of: r, g, b, a, 0, 1
// if 0 or 1, do channel = x * 0 + 1 (before optimization passes)
// [rrggbbaa] = 4 x 2-bit values where:
// 00 = source r
// 01 = source g
// 10 = source b
// 11 = source a
// for ex: [11000110] = X.argb

// static const char *VALID_OPERATORS[] = {
//     "+", "-", "!", "^", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "||", "."
// };

// static const char *VALID_FUNCTIONS[] = {
//     "vec", // vec(A, B, C, D) = A.r, B.g, C.b, D.a
//     "rot", // rot(X, 1) = X.gbar, rot(X, 2) = X.barg, rot(X, 3) = X.argb
//     "lerp",
//     "min",
//     "max",
//     "clamp",
//     "cos",
//     "sin",
//     "exp",
//     "sqrt",
//     "rsqrt",
//     "round",
//     "floor",
//     "ceil",
// };


#define CC_EXT_FLAG_OP                      0x80
#define CC_EXT_FLAG_OP_FUNCTION             0x40
#define CC_EXT_FLAG_VAL_DOT_PARAM           0x40

#define CC_EXT_OP_PRECEDENCE_BITS           0x38
#define CC_EXT_OP_WITH_PRECEDENCE(x, p)     ((x) | CC_EXT_FLAG_OP | ((p) << 3))
#define CC_EXT_OP_FUNCTION(x)               ((x) | CC_EXT_FLAG_OP | CC_EXT_FLAG_OP_FUNCTION)

#define CC_EXT_VAL_DOT_PARAM(op, x)         ((op >> (3 * (x))) & 7)


// VALUES
// 0xyyyyyy
// x = 0 -> yyyyyy from 0 to 63 -> regular constant
// x = 1 -> 2 x 3 bits for dot param

// Note: the order does not follow the CCS constants!
// COMBINED sources are also missing, since there is no cycle 2 in extended combiners

#define CC_EXT_VAL_0                    0
#define CC_EXT_VAL_1                    1
#define CC_EXT_VAL_TEXEL0               2
#define CC_EXT_VAL_TEXEL0A              3
#define CC_EXT_VAL_TEXEL1               4
#define CC_EXT_VAL_TEXEL1A              5
#define CC_EXT_VAL_PRIM                 6
#define CC_EXT_VAL_PRIMA                7
#define CC_EXT_VAL_SHADE                8
#define CC_EXT_VAL_SHADEA               9
#define CC_EXT_VAL_ENV                  10
#define CC_EXT_VAL_ENVA                 11
#define CC_EXT_VAL_LOD                  12
#define CC_EXT_VAL_NOISE                13

#define CC_EXT_VAL_CONST_FIRST          14
#define CC_EXT_VAL_CONST_LAST           (CC_EXT_FLAG_VAL_DOT_PARAM - 1)

// OPS
// 10xxxyyy -> xxx = precedence from 0 to 7, yyy = op from 0 to 7
// 11xxxxxx -> xxxxxx = function id from 0 to 60 (last 3 are reserved for parsing)

#define CC_EXT_OP_DOT                   CC_EXT_OP_WITH_PRECEDENCE(0, 0)

#define CC_EXT_OP_NEG                   CC_EXT_OP_WITH_PRECEDENCE(0, 1)
#define CC_EXT_OP_NOT                   CC_EXT_OP_WITH_PRECEDENCE(1, 1)

#define CC_EXT_OP_POWER                 CC_EXT_OP_WITH_PRECEDENCE(0, 2)

#define CC_EXT_OP_MULT                  CC_EXT_OP_WITH_PRECEDENCE(0, 3)
#define CC_EXT_OP_DIV                   CC_EXT_OP_WITH_PRECEDENCE(1, 3)
#define CC_EXT_OP_MODULO                CC_EXT_OP_WITH_PRECEDENCE(2, 3)

#define CC_EXT_OP_ADD                   CC_EXT_OP_WITH_PRECEDENCE(0, 4)
#define CC_EXT_OP_SUB                   CC_EXT_OP_WITH_PRECEDENCE(1, 4)

#define CC_EXT_OP_LOWER                 CC_EXT_OP_WITH_PRECEDENCE(0, 5)
#define CC_EXT_OP_LOWER_OR_EQUAL        CC_EXT_OP_WITH_PRECEDENCE(1, 5)
#define CC_EXT_OP_GREATER               CC_EXT_OP_WITH_PRECEDENCE(2, 5)
#define CC_EXT_OP_GREATER_OR_EQUAL      CC_EXT_OP_WITH_PRECEDENCE(3, 5)
#define CC_EXT_OP_EQUAL                 CC_EXT_OP_WITH_PRECEDENCE(4, 5)
#define CC_EXT_OP_NOT_EQUAL             CC_EXT_OP_WITH_PRECEDENCE(5, 5)

#define CC_EXT_OP_AND                   CC_EXT_OP_WITH_PRECEDENCE(0, 6)
#define CC_EXT_OP_OR                    CC_EXT_OP_WITH_PRECEDENCE(1, 6)

#define CC_EXT_OP_VEC                   CC_EXT_OP_FUNCTION(0x00)
#define CC_EXT_OP_LERP                  CC_EXT_OP_FUNCTION(0x01)
#define CC_EXT_OP_MIN                   CC_EXT_OP_FUNCTION(0x02)
#define CC_EXT_OP_MAX                   CC_EXT_OP_FUNCTION(0x03)
#define CC_EXT_OP_CLAMP                 CC_EXT_OP_FUNCTION(0x04)
#define CC_EXT_OP_SATURATE              CC_EXT_OP_FUNCTION(0x05)
#define CC_EXT_OP_COS                   CC_EXT_OP_FUNCTION(0x06)
#define CC_EXT_OP_SIN                   CC_EXT_OP_FUNCTION(0x07)
#define CC_EXT_OP_EXP                   CC_EXT_OP_FUNCTION(0x08)
#define CC_EXT_OP_SQRT                  CC_EXT_OP_FUNCTION(0x09)
#define CC_EXT_OP_RSQRT                 CC_EXT_OP_FUNCTION(0x0A)
#define CC_EXT_OP_ROUND                 CC_EXT_OP_FUNCTION(0x0B)
#define CC_EXT_OP_FLOOR                 CC_EXT_OP_FUNCTION(0x0C)
#define CC_EXT_OP_CEIL                  CC_EXT_OP_FUNCTION(0x0D)

// SPECIAL

#define CC_EXT_SPECIAL_OPEN             0xFD
#define CC_EXT_SPECIAL_CLOSE            0xFE
#define CC_EXT_SPECIAL_COMMA            0xFF


static inline bool op_is_special(uint8_t op) {
    return op == CC_EXT_SPECIAL_OPEN ||
           op == CC_EXT_SPECIAL_CLOSE ||
           op == CC_EXT_SPECIAL_COMMA;
}

static inline bool op_is_operator(uint8_t op) {
    return !op_is_special(op) && (op & CC_EXT_FLAG_OP) != 0;
}

static inline bool op_is_function(uint8_t op) {
    return op_is_operator(op) && (op & CC_EXT_FLAG_OP_FUNCTION) != 0;
}

static inline bool op_is_value(uint8_t op) {
    return (op & CC_EXT_FLAG_OP) == 0;
}

static inline bool op_is_dot_param(uint8_t op) {
    return op_is_value(op) && (op & CC_EXT_FLAG_VAL_DOT_PARAM) != 0;
}

static inline bool op_can_be_value(uint8_t op) {
    return op_is_value(op) || op == CC_EXT_SPECIAL_CLOSE;
}

static inline bool op_can_be_operator(uint8_t op) {
    return op_is_operator(op) || op == CC_EXT_SPECIAL_OPEN || op == CC_EXT_SPECIAL_COMMA;
}





static const char *OP_TO_STR[256] = {
    [CC_EXT_VAL_0] = "VAL_0",
    [CC_EXT_VAL_1] = "VAL_1",
    [CC_EXT_VAL_TEXEL0] = "VAL_TEXEL0",
    [CC_EXT_VAL_TEXEL0A] = "VAL_TEXEL0A",
    [CC_EXT_VAL_TEXEL1] = "VAL_TEXEL1",
    [CC_EXT_VAL_TEXEL1A] = "VAL_TEXEL1A",
    [CC_EXT_VAL_NOISE] = "VAL_NOISE",
    [CC_EXT_VAL_PRIM] = "VAL_PRIM",
    [CC_EXT_VAL_PRIMA] = "VAL_PRIMA",
    [CC_EXT_VAL_SHADE] = "VAL_SHADE",
    [CC_EXT_VAL_SHADEA] = "VAL_SHADEA",
    [CC_EXT_VAL_ENV] = "VAL_ENV",
    [CC_EXT_VAL_ENVA] = "VAL_ENVA",
    [CC_EXT_VAL_LOD] = "VAL_LOD",
    [CC_EXT_OP_DOT] = "OP_DOT",
    [CC_EXT_OP_NEG] = "OP_NEG",
    [CC_EXT_OP_NOT] = "OP_NOT",
    [CC_EXT_OP_POWER] = "OP_POWER",
    [CC_EXT_OP_MULT] = "OP_MULT",
    [CC_EXT_OP_DIV] = "OP_DIV",
    [CC_EXT_OP_MODULO] = "OP_MODULO",
    [CC_EXT_OP_ADD] = "OP_ADD",
    [CC_EXT_OP_SUB] = "OP_SUB",
    [CC_EXT_OP_LOWER] = "OP_LOWER",
    [CC_EXT_OP_LOWER_OR_EQUAL] = "OP_LOWER_OR_EQUAL",
    [CC_EXT_OP_GREATER] = "OP_GREATER",
    [CC_EXT_OP_GREATER_OR_EQUAL] = "OP_GREATER_OR_EQUAL",
    [CC_EXT_OP_EQUAL] = "OP_EQUAL",
    [CC_EXT_OP_NOT_EQUAL] = "OP_NOT_EQUAL",
    [CC_EXT_OP_AND] = "OP_AND",
    [CC_EXT_OP_OR] = "OP_OR",
    [CC_EXT_OP_VEC] = "OP_VEC",
    [CC_EXT_OP_LERP] = "OP_LERP",
    [CC_EXT_OP_MIN] = "OP_MIN",
    [CC_EXT_OP_MAX] = "OP_MAX",
    [CC_EXT_OP_CLAMP] = "OP_CLAMP",
    [CC_EXT_OP_SATURATE] = "OP_SATURATE",
    [CC_EXT_OP_COS] = "OP_COS",
    [CC_EXT_OP_SIN] = "OP_SIN",
    [CC_EXT_OP_EXP] = "OP_EXP",
    [CC_EXT_OP_SQRT] = "OP_SQRT",
    [CC_EXT_OP_RSQRT] = "OP_RSQRT",
    [CC_EXT_OP_ROUND] = "OP_ROUND",
    [CC_EXT_OP_FLOOR] = "OP_FLOOR",
    [CC_EXT_OP_CEIL] = "OP_CEIL",
};






#define EXTENDED_COMBINER_MAX_OPS           128
#define EXTENDED_COMBINER_MAX_CONSTANTS     (CC_EXT_VAL_CONST_LAST - CC_EXT_VAL_CONST_FIRST + 1)

struct ExtendedCombiner {
    uint8_t ops[EXTENDED_COMBINER_MAX_OPS];
    uint32_t num_ops;
    float constants[EXTENDED_COMBINER_MAX_CONSTANTS][4];
    uint32_t num_constants;
};



typedef struct Token {
    char *str;
    size_t pos;
    size_t len;
    struct Token *prev;
} Token;

typedef struct Op {
    uint8_t op;
    const Token *token;
} Op;




static void free_token(void *ptr) {
    Token *token = ptr;
    if (token) {
        free(token->str);
        free(token);
    }
}


static void push_token(struct GrowingArray *tokens, const char *src, size_t pos, size_t len) {
    if (len > 0) {
        Token *token = growing_array_alloc(tokens, sizeof(Token));
        token->str = memcpy(calloc(1, len + 1), src, len);
        token->pos = pos;
        token->len = len;
    }
}


#define push_curr_token() { \
    push_token(tokens, curr_token, curr_start, curr_len); \
    curr_len = 0; \
    is_number = false; \
    is_decimal = false; \
    is_dot_arg = false; \
}




static inline void push_op(struct GrowingArray *ops, uint8_t op, const Token *token) {
    Op *new_op = growing_array_alloc(ops, sizeof(Op));
    new_op->op = op;
    new_op->token = token;
}

#define token_is(_s_) \
    stricmp(token->str, _s_) == 0

#define token_to_op(_s_, _op_, _is_unary_) { \
    if (token_is(_s_)) { \
        if (( (_is_unary_) && op_can_be_operator(prev)) || \
            (!(_is_unary_) && op_can_be_value(prev))) { \
            push_op(ops, (_op_), token); \
        } else { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), "Operator `%s` at position %llu must follow %s.\n", _s_, token->pos, (_is_unary_) ? "an operator" : "a value"); \
            print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        continue; \
    } \
}

#define token_to_function(_s_, _op_) { \
    if (token_is(_s_)) { \
        const Token *next_token = (_head_ + 1 < _tail_ ? *(_head_ + 1) : NULL); \
        if (!next_token || stricmp(next_token->str, "(") != 0) { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), "Function `%s` at position %llu must be followed by a left parenthesis `(`.\n", _s_, token->pos); \
            print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        if (op_can_be_operator(prev)) { \
            push_op(ops, (_op_), token); \
        } else { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), "Function `%s` at position %llu must follow an operator.\n", _s_, token->pos); \
            print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        continue; \
    } \
}

#define token_to_value(_s_, _op_) { \
    if (token_is(_s_)) { \
        if (op_can_be_operator(prev)) { \
            push_op(ops, (_op_), token); \
        } else { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), "Value `%s` at position %llu must follow an operator.\n", _s_, token->pos); \
            print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        continue; \
    } \
}


static inline Op *get_op(struct GrowingArray *ops, uint32_t index) {
    return (Op *) ops->buffer[index];
}

static Op *get_op_from_token(struct GrowingArray *ops, const Token *token) {
    if (!token) { return NULL; }
    growing_array_for_each_(ops, Op, op) {
        if (op->token == token) {
            return op;
        }
    }
    return NULL;
}

static int get_op_index(struct GrowingArray *ops, const Op *op) {
    if (!op) { return -1; }
    for (uint32_t i = 0; i < ops->count; ++i) {
        if (get_op(ops, i) == op) {
            return i;
        }
    }
    return -1;
}



static inline void push_constant(struct GrowingArray *constants, float constant) {
    *((float *) growing_array_alloc(constants, sizeof(float))) = constant;
}

static inline float get_constant(struct GrowingArray *constants, uint32_t index) {
    return *((float *) constants->buffer[index]);
}




static bool check_number(const Token *token) {
    size_t len = strlen(token->str);
    bool decimal = false;
    for (size_t i = 0; i < len; ++i) {
        char c = token->str[i];
        if (c == '.') {
            if (decimal) {
                return false;
            }
            decimal = true;
            continue;
        }
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

static int get_constant_index(struct GrowingArray *constants, float value) {
    for (uint32_t i = 0; i < constants->count; ++i) {
        if (get_constant(constants, i) == value) {
            return i;
        }
    }
    return -1;
}


static inline bool is_right_associative(uint8_t op) {
    return op == CC_EXT_OP_POWER;
}

static inline void stack_push(struct GrowingArray *stack, Op *op) {
    push_op(stack, op->op, op->token);
}

static inline Op *stack_top(struct GrowingArray *stack) {
    return get_op(stack, stack->count - 1);
}

static inline Op *stack_pop(struct GrowingArray *stack) {
    Op *op = stack_top(stack);
    stack->count--;
    return op;
}


static struct GrowingArray *infix_to_prefix(struct GrowingArray *ops) {
    struct GrowingArray *res = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *stack = growing_array_init(NULL, 16, malloc, free);
    bool ok = false;

    // from right to left
    for (int i = ops->count - 1; i >= 0; --i) {
        Op *op = get_op(ops, i);

        if (op_is_value(op->op)) {
            push_op(res, op->op, op->token);
            continue;
        }

        if (op->op == CC_EXT_SPECIAL_CLOSE) {
            stack_push(stack, op);
            continue;
        }

        if (op->op == CC_EXT_SPECIAL_OPEN) {
            while (stack->count > 0 && stack_top(stack)->op != CC_EXT_SPECIAL_CLOSE) {
                Op *top = stack_pop(stack);
                push_op(res, top->op, top->token);
            }

            if (i > 0) {
                Op *prev = get_op(ops, i - 1);
                if (op_is_function(prev->op)) {
                    push_op(res, prev->op, prev->token);
                    i--;
                }
            }

            // should pop ')'
            if (stack->count == 0 || stack_top(stack)->op != CC_EXT_SPECIAL_CLOSE) {
                printf("ERROR: Missing ')'\n");
                goto cleanup;
            } else {
                stack_pop(stack);
            }
            continue;
        }

        if (op_is_operator(op->op)) {
            Op *top;
            while (
                stack->count > 0 &&
                (top = stack_top(stack)) &&
                op_is_operator(top->op) && (
                (top->op & CC_EXT_OP_PRECEDENCE_BITS) < (op->op & CC_EXT_OP_PRECEDENCE_BITS) || (
                (top->op & CC_EXT_OP_PRECEDENCE_BITS) == (op->op & CC_EXT_OP_PRECEDENCE_BITS) && is_right_associative(op->op)
            ))) {
                top = stack_pop(stack);
                push_op(res, top->op, top->token);
            }
            stack_push(stack, op);
            continue;
        }
    }

    // pop remaining ops
    while (stack->count > 0) {
        Op *op = stack_pop(stack);
        push_op(res, op->op, op->token);
    }
    ok = true;

cleanup:
    growing_array_free(&stack);
    if (!ok) {
        growing_array_free(&res);
        return NULL;
    }
    return res;
}


static void print_ops(struct GrowingArray *ops, struct GrowingArray *constants) {
    growing_array_for_each_(ops, const Op, op) {
        if (op_is_operator(op->op)) {
            printf("%s ", OP_TO_STR[op->op]);
            continue;
        }

        if (op_is_dot_param(op->op)) {
            for (int j = 0; j < 2; ++j) {
                printf("%c", "rgba01"[CC_EXT_VAL_DOT_PARAM(op->op, j)]);
            }
            printf(" ");
            continue;
        }

        if (op_is_value(op->op)) {
            if (op->op >= CC_EXT_VAL_CONST_FIRST) {
                printf("[%d]%f ", op->op - CC_EXT_VAL_CONST_FIRST, get_constant(constants, op->op - CC_EXT_VAL_CONST_FIRST));
            } else {
                printf("%s ", OP_TO_STR[op->op]);
            }
            continue;
        }

        switch (op->op) {
            case CC_EXT_SPECIAL_OPEN:  printf("( "); break;
            case CC_EXT_SPECIAL_CLOSE: printf(") "); break;
            case CC_EXT_SPECIAL_COMMA: printf(", "); break;
        }
    }
    printf("\n");
}





static void print_error_location(char *error_msg, size_t msg_len, const char *str, size_t pos, size_t len, size_t offset) {
    size_t curr_len = strlen(error_msg);
    error_msg += curr_len;
    msg_len -= curr_len;

    size_t written_chars = snprintf(error_msg, msg_len, "%s\n%*s", str, (int) pos, "");
    error_msg += written_chars;
    msg_len -= written_chars;

    size_t index = 0;
    while (msg_len > 1 && index < len) {
        *(error_msg++) = (index == offset ? '^' : '~');
        msg_len--;
        index++;
    }

    *error_msg = 0;
}



static const char *parse_combiner(const char *str, struct ExtendedCombiner *comb) {
    static char sErrorMsg[1024];

    struct GrowingArray *tokens    = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *ops       = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *constants = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *prefix    = NULL;
    bool ok = false;

    // tokenize input
    bool is_number = false;
    bool is_decimal = false;
    bool is_dot_arg = false;
    char curr_token[32];
    size_t curr_start = 0;
    size_t curr_len = 0;
    size_t len = strlen(str);
    for (size_t i = 0; i <= len; ++i) {
        if (i == len) {
            push_curr_token();
            break;
        }

        char c = str[i];
        char c2 = str[i + 1];

        // ignore whitespaces and others
        if (iscntrl(c) || isspace(c)) {
            push_curr_token();
            continue;
        }

        // double chars symbols
        if ((c == '<' && c2 == '=') ||
            (c == '>' && c2 == '=') ||
            (c == '=' && c2 == '=') ||
            (c == '!' && c2 == '=') ||
            (c == '&' && c2 == '&') ||
            (c == '|' && c2 == '|')) {
            push_curr_token();
            push_token(tokens, str + i, i, 2);
            i++;
            continue;
        }

        // single char symbols
        if (c == '+' || c == '-' || c == '!' || c == '^' || c == '*' ||
            c == '/' || c == '%' || c == '<' || c == '>' || (c == '.' && !is_number) ||
            c == '(' || c == ')' || c == ',') {
            push_curr_token();
            push_token(tokens, str + i, i, 1);
            is_dot_arg = (c == '.');
            continue;
        }

        // check token length
        if (curr_len == sizeof(curr_token)) {
            snprintf(sErrorMsg, sizeof(sErrorMsg), "Token at position %llu is too long.\n", i - curr_len);
            print_error_location(sErrorMsg, sizeof(sErrorMsg), str, i - curr_len, curr_len, 0);
            goto cleanup;
        }

        // number
        if (!is_dot_arg && isdigit(c) && (is_number || curr_len == 0)) {
            if (curr_len == 0) { curr_start = i; }
            curr_token[curr_len++] = c;
            is_number = true;
            continue;
        }
        if (is_number && c == '.') {
            if (is_decimal) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Number at position %llu cannot have two decimal points.\n", i - curr_len);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, i - curr_len, curr_len + 1, curr_len);
                goto cleanup;
            }
            if (curr_len == 0) { curr_start = i; }
            curr_token[curr_len++] = c;
            is_decimal = true;
            continue;
        }

        // named value
        if (!is_number && (isalpha(c) || isdigit(c) || c == '_')) {
            if (curr_len == 0) { curr_start = i; }
            curr_token[curr_len++] = c;
            continue;
        }

        // invalid char
        snprintf(sErrorMsg, sizeof(sErrorMsg), "Invalid character at position %llu:  %c  (%d, 0x%02X)\n", i, c, c, c);
        print_error_location(sErrorMsg, sizeof(sErrorMsg), str, i, 1, 0);
        goto cleanup;
    }


    // printf("===============================\n");
    // printf("tokens: %u\n", tokens->count);
    // growing_array_for_each_(tokens, const Token, token) {
    //     printf("%s\n", token->str);
    // }
    // printf("===============================\n");


    // turn tokens into ops
    Token *scope = NULL;
    growing_array_for_each_(tokens, Token, token) {
        uint8_t prev = (ops->count > 0 ? get_op(ops, ops->count - 1)->op : CC_EXT_SPECIAL_COMMA);

        // symbols
        token_to_op("!", CC_EXT_OP_NOT, true);
        token_to_op("^", CC_EXT_OP_POWER, false);
        token_to_op("*", CC_EXT_OP_MULT, false);
        token_to_op("/", CC_EXT_OP_DIV, false);
        token_to_op("%", CC_EXT_OP_MODULO, false);
        token_to_op("<", CC_EXT_OP_LOWER, false);
        token_to_op("<=", CC_EXT_OP_LOWER_OR_EQUAL, false);
        token_to_op(">", CC_EXT_OP_GREATER, false);
        token_to_op(">=", CC_EXT_OP_GREATER_OR_EQUAL, false);
        token_to_op("==", CC_EXT_OP_EQUAL, false);
        token_to_op("!=", CC_EXT_OP_NOT_EQUAL, false);
        token_to_op("&&", CC_EXT_OP_AND, false);
        token_to_op("||", CC_EXT_OP_OR, false);
        token_to_op(".", CC_EXT_OP_DOT, false);

        if (token_is("+")) {
            if (op_can_be_value(prev)) {
                push_op(ops, CC_EXT_OP_ADD, token);
            } else {
                // ignore unary plus
            }
            continue;
        }
        if (token_is("-")) {
            if (op_can_be_value(prev)) {
                push_op(ops, CC_EXT_OP_SUB, token);
            } else {
                push_op(ops, CC_EXT_OP_NEG, token);
            }
            continue;
        }
        if (token_is("(")) {
            token->prev = scope;
            scope = token;
            if (op_can_be_operator(prev)) {
                push_op(ops, CC_EXT_SPECIAL_OPEN, token);
            } else {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Left parenthesis `(` at position %llu must follow an operator.\n", token->pos);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            continue;
        }
        if (token_is(")")) {
            if (!op_can_be_value(prev)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Right parenthesis `)` at position %llu must follow a value.\n", token->pos);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            if (!scope) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Too much right parentheses `)` at position %llu.\n", token->pos);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            push_op(ops, CC_EXT_SPECIAL_CLOSE, token);
            scope = scope->prev;
            continue;
        }
        if (token_is(",")) {
            if (!op_can_be_value(prev)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Function parameter separator `,` at position %llu must follow a value.\n", token->pos);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            // check if inside function
            // get current scope, check previous op
            const Op *op = get_op_from_token(ops, scope);
            int index = get_op_index(ops, op);
            if (index <= 0 || !op_is_function(get_op(ops, index - 1)->op)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Function parameter separator `,` at position %llu cannot be used outside a function.\n", token->pos);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            push_op(ops, CC_EXT_SPECIAL_COMMA, token);
            continue;
        }

        // functions
        token_to_function("vec", CC_EXT_OP_VEC);
        token_to_function("lerp", CC_EXT_OP_LERP);
        token_to_function("min", CC_EXT_OP_MIN);
        token_to_function("max", CC_EXT_OP_MAX);
        token_to_function("clamp", CC_EXT_OP_CLAMP);
        token_to_function("saturate", CC_EXT_OP_SATURATE);
        token_to_function("cos", CC_EXT_OP_COS);
        token_to_function("sin", CC_EXT_OP_SIN);
        token_to_function("exp", CC_EXT_OP_EXP);
        token_to_function("sqrt", CC_EXT_OP_SQRT);
        token_to_function("rsqrt", CC_EXT_OP_RSQRT);
        token_to_function("round", CC_EXT_OP_ROUND);
        token_to_function("floor", CC_EXT_OP_FLOOR);
        token_to_function("ceil", CC_EXT_OP_CEIL);

        // values
        token_to_value("0", CC_EXT_VAL_0);
        token_to_value("0.0", CC_EXT_VAL_0);
        token_to_value("1", CC_EXT_VAL_1);
        token_to_value("1.0", CC_EXT_VAL_1);
        token_to_value("TEXEL0", CC_EXT_VAL_TEXEL0);
        token_to_value("TEXEL0A", CC_EXT_VAL_TEXEL0A);
        token_to_value("TEXEL0_ALPHA", CC_EXT_VAL_TEXEL0A);
        token_to_value("TEXEL1", CC_EXT_VAL_TEXEL1);
        token_to_value("TEXEL1A", CC_EXT_VAL_TEXEL1A);
        token_to_value("TEXEL1_ALPHA", CC_EXT_VAL_TEXEL1A);
        token_to_value("NOISE", CC_EXT_VAL_NOISE);
        token_to_value("PRIM", CC_EXT_VAL_PRIM);
        token_to_value("PRIMITIVE", CC_EXT_VAL_PRIM);
        token_to_value("PRIMA", CC_EXT_VAL_PRIMA);
        token_to_value("PRIMITIVE_ALPHA", CC_EXT_VAL_PRIMA);
        token_to_value("SHADE", CC_EXT_VAL_SHADE);
        token_to_value("SHADEA", CC_EXT_VAL_SHADEA);
        token_to_value("SHADE_ALPHA", CC_EXT_VAL_SHADEA);
        token_to_value("ENV", CC_EXT_VAL_ENV);
        token_to_value("ENVIRONMENT", CC_EXT_VAL_ENV);
        token_to_value("ENVA", CC_EXT_VAL_ENVA);
        token_to_value("ENV_ALPHA", CC_EXT_VAL_ENVA);
        token_to_value("LOD", CC_EXT_VAL_LOD);
        token_to_value("LOD_FRACTION", CC_EXT_VAL_LOD);

        // dot
        if (ops->count > 0 && prev == CC_EXT_OP_DOT) {
            size_t len = strlen(token->str);
            if (len != 1 && len != 3 && len != 4) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Operator `.` parameter `%s` at position %llu has invalid length: %llu (must be 1, 3 or 4).\n", token->str, token->pos, len);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            uint8_t dot_params[2] = { CC_EXT_FLAG_VAL_DOT_PARAM, CC_EXT_FLAG_VAL_DOT_PARAM };
            for (size_t i = 0; i < 4; ++i) {
                char c = (
                    i < len ? token->str[i] : (
                    len == 1 ? token->str[0] : (
                    len == 3 ? '1' : '0'
                )));
                uint8_t k = 0;
                switch (c) {
                    case 'r': case 'R': k = 0; break;
                    case 'g': case 'G': k = 1; break;
                    case 'b': case 'B': k = 2; break;
                    case 'a': case 'A': k = 3; break;
                    case '0':           k = 4; break;
                    case '1':           k = 5; break;
                    default:
                        snprintf(sErrorMsg, sizeof(sErrorMsg), "Operator `.` parameter `%s` at position %llu has invalid character:  %c  (%d, 0x%02X) (must be one of \"rgba01\").\n", token->str, token->pos, c, c, c);
                        print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, i);
                        goto cleanup;
                }
                dot_params[i / 2] |= (k << (3 * (i % 2)));
            }

            push_op(ops, dot_params[0], token);
            push_op(ops, dot_params[1], token);
            continue;
        }

        // numbers and math constants
        float constant;
        if ((check_number(token) && sscanf(token->str, "%f", &constant) == 1) ||
            (token_is("PI") && (constant = M_PI)) ||
            (token_is("E") && (constant = M_E))) {
            if (!op_can_be_operator(prev)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Constant `%f` at position %llu must follow an operator.\n", constant, token->pos);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            // handle 0 and 1
            if (constant == 0) {
                push_op(ops, CC_EXT_VAL_0, token);
                continue;
            }
            if (constant == 1) {
                push_op(ops, CC_EXT_VAL_1, token);
                continue;
            }

            // don't duplicate constants
            int index = get_constant_index(constants, constant);
            if (index >= 0) {
                push_op(ops, CC_EXT_VAL_CONST_FIRST + index, token);
            } else if (constants->count < EXTENDED_COMBINER_MAX_CONSTANTS) {
                push_op(ops, CC_EXT_VAL_CONST_FIRST + constants->count, token);
                push_constant(constants, constant);
            } else {
                snprintf(sErrorMsg, sizeof(sErrorMsg), "Max number of constants reached: %u\n", EXTENDED_COMBINER_MAX_CONSTANTS);
                print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            continue;
        }

        // invalid
        snprintf(sErrorMsg, sizeof(sErrorMsg), "Invalid token at position %llu: `%s`\n", token->pos, token->str);
        print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
        goto cleanup;
    }

    // check for missing right parentheses
    if (scope) {
        snprintf(sErrorMsg, sizeof(sErrorMsg), "Unclosed left parenthesis `(` at position %llu.\n", scope->pos);
        print_error_location(sErrorMsg, sizeof(sErrorMsg), str, scope->pos, scope->len, 0);
        goto cleanup;
    }

    print_ops(ops, constants);
    printf("\n");

    // prefix = infix_to_prefix(ops);
    // if (!prefix) {
    //     goto cleanup;
    // }

    // print_ops(prefix, constants);
    // printf("\n");

    // // validate


    // // optimize


    // // check sizes
    // if (prefix->count > EXTENDED_COMBINER_MAX_OPS) {
    //     printf("Max number of ops exceeded: %d > %d\n", prefix->count, EXTENDED_COMBINER_MAX_OPS);
    //     goto cleanup;
    // }
    // if (constants->count > EXTENDED_COMBINER_MAX_CONSTANTS) {
    //     printf("Max number of constants exceeded: %d > %d\n", constants->count, EXTENDED_COMBINER_MAX_CONSTANTS);
    //     goto cleanup;
    // }

    // // fill struct
    // memset(comb, 0, sizeof(*comb));
    // comb->num_ops = prefix->count;
    // for (uint32_t i = 0; i < prefix->count; ++i) {
    //     comb->ops[i] = get_op(prefix, i);
    // }
    // comb->num_constants = constants->count;
    // for (uint32_t i = 0; i < constants->count; ++i) {
    //     comb->constants[i][0] = get_constant(constants, i);
    // }
    ok = true;

cleanup:
    growing_array_free(&tokens);
    growing_array_free(&ops);
    growing_array_free(&constants);
    growing_array_free(&prefix);
    return ok ? NULL : sErrorMsg;
}


static const char *error_checks[] = {
"TEXEL0 ! TEXEL1",
"TEXEL0 + * TEXEL1",
"lerp -TEXEL0, TEXEL1, ENV",
"(TEXEL0) TEXEL1",
"TEXEL00000000000000000000000000000000000",
"12.5.2",
"[]",
"TEXEL0 (TEXEL1)",
"(TEXEL0 + )",
"(TEXEL0))",
"lerp(, TEXEL0)",
"(TEXEL0, TEXEL1)",
"lerp(TEXEL0, (TEXEL1, ENV))",
"TEXEL0.rg",
"TEXEL0.abcd",
"TEXEL0 1.25",
"TEXEL2",
"lerp(TEXEL0",


};




__attribute__((constructor)) static void test_custom_combine() {

    const char *comb_str = 
    "ENV * (-TEXEL0 + TEXEL1.a).gb1 + -0.5 + 0.2 + vec(9, 10, 11, 12) - min(0.5, max(lerp(PRIM + ENVA * 0.75, SHADE, PRIMA).00ra.gbra, 0)) + 2 * 1 - 1.5 ^ (3 + 1)";


    // for (int i = 0; i < ARRAY_COUNT(error_checks); ++i) {

    // const char *comb_str = error_checks[i];

    struct ExtendedCombiner comb;
    const char *err = parse_combiner(comb_str, &comb);

    // printf("\n");
    if (err) {
        printf("%s\n\n", err);
    }

    // }


    exit(0);
}

