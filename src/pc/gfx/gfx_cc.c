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


#define CC_EXT_FLAG_OP                      0x80
#define CC_EXT_FLAG_VAL_DOT_PARAM           0x40

#define CC_EXT_VAL_DOT_PARAM(op, x)         (((op) >> (3 * (x))) & 0x7)

#define CC_EXT_OP_ARGS_SHIFT                5
#define CC_EXT_OP_ARGS_WIDTH                2



// Values


enum {
#define CC_EXT_VAL(name) CC_EXT_VAL_ID_##name,
#include "gfx_cc_ext_val.inl"
#undef CC_EXT_VAL
};


// Operators and functions

enum {
#define CC_EXT_OP_1(name) CC_EXT_OP_ID_##name,
#define CC_EXT_OP_2(name)
#define CC_EXT_OP_3(name) CC_EXT_OP_ID_##name,
#define CC_EXT_OP(name, args, ...) CC_EXT_OP_##args(name)
#include "gfx_cc_ext_op.inl"
#undef CC_EXT_OP_1
#undef CC_EXT_OP_2
#undef CC_EXT_OP_3
#undef CC_EXT_OP
#define CC_EXT_FUNC_1(name) CC_EXT_FUNC_ID_##name,
#define CC_EXT_FUNC_2(name)
#define CC_EXT_FUNC_3(name) CC_EXT_FUNC_ID_##name,
#define CC_EXT_FUNC_4(name)
#define CC_EXT_FUNC(name, args, ...) CC_EXT_FUNC_##args(name)
#include "gfx_cc_ext_func.inl"
#undef CC_EXT_FUNC_1
#undef CC_EXT_FUNC_2
#undef CC_EXT_FUNC_3
#undef CC_EXT_FUNC_4
#undef CC_EXT_FUNC
};

enum {
#define CC_EXT_OP_1(name)
#define CC_EXT_OP_2(name) CC_EXT_OP_ID_##name,
#define CC_EXT_OP_3(name)
#define CC_EXT_OP(name, args, ...) CC_EXT_OP_##args(name)
#include "gfx_cc_ext_op.inl"
#undef CC_EXT_OP_1
#undef CC_EXT_OP_2
#undef CC_EXT_OP_3
#undef CC_EXT_OP
#define CC_EXT_FUNC_1(name)
#define CC_EXT_FUNC_2(name) CC_EXT_FUNC_ID_##name,
#define CC_EXT_FUNC_3(name)
#define CC_EXT_FUNC_4(name) CC_EXT_FUNC_ID_##name,
#define CC_EXT_FUNC(name, args) CC_EXT_FUNC_##args(name)
#include "gfx_cc_ext_func.inl"
#undef CC_EXT_FUNC_1
#undef CC_EXT_FUNC_2
#undef CC_EXT_FUNC_3
#undef CC_EXT_FUNC_4
#undef CC_EXT_FUNC
};





enum {

// Values
#define CC_EXT_VAL(name) CC_EXT_VAL_##name = CC_EXT_VAL_ID_##name,
#include "gfx_cc_ext_val.inl"
#undef CC_EXT_VAL
    CC_EXT_VAL_CONST_FIRST,
    CC_EXT_VAL_CONST_LAST = (CC_EXT_FLAG_VAL_DOT_PARAM - 1),

// Operators and functions
#define CC_EXT_OP(name, args, ...)  CC_EXT_OP_##name   = (CC_EXT_OP_ID_##name   | ((args - 1) << CC_EXT_OP_ARGS_SHIFT) | CC_EXT_FLAG_OP),
#define CC_EXT_FUNC(name, args)     CC_EXT_FUNC_##name = (CC_EXT_FUNC_ID_##name | ((args - 1) << CC_EXT_OP_ARGS_SHIFT) | CC_EXT_FLAG_OP),
#include "gfx_cc_ext_op.inl"
#include "gfx_cc_ext_func.inl"
#undef CC_EXT_OP
#undef CC_EXT_FUNC

// Special ops for parsing
    CC_EXT_SPECIAL_OPEN  = 0xFD,
    CC_EXT_SPECIAL_CLOSE = 0xFE,
    CC_EXT_SPECIAL_COMMA = 0xFF,
};


// // OPS, num args is given by precedence
// // 8 x 2-bits, from precedence 7 to 0: 2 2 2 2 2 2 1 3 = 0b1010101010100111 = 0xAAA7
// #define CC_EXT_OP_NUM_ARGS_BITS  0xAAA7
// #define CC_EXT_OP_NUM_ARGS(op) \
//     ((CC_EXT_OP_NUM_ARGS_BITS >> (2 * CC_EXT_OP_PRECEDENCE(op))) & 0x3)

// // FUNCTIONS, num args is given by function index
// // 4 x 8 x 2-bits, from index 15/31/47/63 to 0/16/32/48

// #define CC_EXT_FUNC_NUM_ARGS_BITS(index, args) \
//     (((args) - 1) << (2 * ((index) % 0x10)))

// static const int CC_EXT_FUNC_00_15_NUM_ARGS = (0
// #define CC_EXT_VAL(...)
// #define CC_EXT_OP(...)
// #define CC_EXT_FUNC(name, index, args) | CC_EXT_FUNC_NUM_ARGS_BITS(index, args)
// #include "gfx_cc_ext.inl"
// #undef CC_EXT_VAL
// #undef CC_EXT_OP
// #undef CC_EXT_FUNC
// );


// #define CC_EXT_FUNC_NUM_ARGS(op) \
//     (((CC_EXT_FUNC_00_15_NUM_ARGS >> (2 * ((op) & 0x3F))) & 0x3) + 1)






static inline bool op_is_special(uint8_t op) {
    return op == CC_EXT_SPECIAL_OPEN ||
           op == CC_EXT_SPECIAL_CLOSE ||
           op == CC_EXT_SPECIAL_COMMA;
}

static inline bool op_is_operator(uint8_t op) {
    return !op_is_special(op) && (op & CC_EXT_FLAG_OP) != 0;
}

static inline bool op_is_function(uint8_t op) {
    static const bool IS_FUNCTION[0x100] = {
#define CC_EXT_FUNC(name, ...) [CC_EXT_FUNC_##name] = true,
#include "gfx_cc_ext_func.inl"
#undef CC_EXT_FUNC
    };
    return IS_FUNCTION[op];
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

static inline uint8_t op_get_precedence(uint8_t op) {
    static const uint8_t OP_PRECEDENCE[0x100] = {
#define CC_EXT_OP(name, args, precedence, ...) [CC_EXT_OP_##name] = precedence,
#include "gfx_cc_ext_op.inl"
#undef CC_EXT_OP
    };
    return OP_PRECEDENCE[op];
}








static const char *OP_TO_STR[0x100] = {
#define CC_EXT_VAL(name)        [CC_EXT_VAL_##name] = "VAL_" #name,
#define CC_EXT_OP(name, ...)    [CC_EXT_OP_##name] = "OP_" #name,
#define CC_EXT_FUNC(name, ...)  [CC_EXT_FUNC_##name] = "FUNC_" #name,
#include "gfx_cc_ext_val.inl"
#include "gfx_cc_ext_op.inl"
#include "gfx_cc_ext_func.inl"
#undef CC_EXT_VAL
#undef CC_EXT_OP
#undef CC_EXT_FUNC
};






#define EXTENDED_COMBINER_MAX_OPS           128
#define EXTENDED_COMBINER_MAX_CONSTANTS     (CC_EXT_VAL_CONST_LAST - CC_EXT_VAL_CONST_FIRST + 1)
#define EXTENDED_COMBINER_STACK_SIZE        32

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

typedef struct Constant {
    float x, y, z, w;
} Constant;




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
    Constant *new_constant = growing_array_alloc(constants, sizeof(Constant));
    new_constant->x = constant;
    new_constant->y = constant;
    new_constant->z = constant;
    new_constant->w = constant;
}

static inline void push_constant_4(struct GrowingArray *constants, float x, float y, float z, float w) {
    Constant *new_constant = growing_array_alloc(constants, sizeof(Constant));
    new_constant->x = x;
    new_constant->y = y;
    new_constant->z = z;
    new_constant->w = w;
}


static inline Constant *get_constant(struct GrowingArray *constants, uint32_t index) {
    return (Constant *) constants->buffer[index];
}

static int get_constant_index(struct GrowingArray *constants, float value) {
    for (uint32_t i = 0; i < constants->count; ++i) {
        if (get_constant(constants, i)->x == value) {
            return i;
        }
    }
    return -1;
}

static int get_constant_4_index(struct GrowingArray *constants, float x, float y, float z, float w) {
    for (uint32_t i = 0; i < constants->count; ++i) {
        const Constant *constant = get_constant(constants, i);
        if (constant->x == x && constant->y == y &&
            constant->z == z && constant->w == w) {
            return i;
        }
    }
    return -1;
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


static inline bool is_right_associative(uint8_t op) {
    return op == CC_EXT_OP_POW;
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




static struct GrowingArray *infix_to_prefix(struct GrowingArray *ops, char *error_msg, size_t msg_len, const char *str) {
    struct GrowingArray *res   = growing_array_init(NULL, 16, malloc, free);
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
                if (op->token) {
                    snprintf(error_msg, msg_len, "Missing right parenthesis `)` matching left parenthesis `(` at position %llu.\n", op->token->pos);
                    print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                } else {
                    snprintf(error_msg, msg_len, "Missing right parenthesis `)`.");
                }
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
                op_get_precedence(top->op) < op_get_precedence(op->op) || (
                op_get_precedence(top->op) == op_get_precedence(op->op) && is_right_associative(op->op)
            ))) {
                top = stack_pop(stack);
                push_op(res, top->op, top->token);
            }
            stack_push(stack, op);
            continue;
        }

        if (op->op != CC_EXT_SPECIAL_COMMA) {
            if (op->token) {
                snprintf(error_msg, msg_len, "Invalid op 0x%02X from token at position %llu: `%s`\n", op->op, op->token->pos, op->token->str);
                print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
            } else {
                snprintf(error_msg, msg_len, "Invalid op 0x%02X.", op->op);
            }
            goto cleanup;
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
                printf("[%d] ", op->op - CC_EXT_VAL_CONST_FIRST);
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
    for (uint32_t i = 0; i < constants->count; ++i) {
        const Constant *constant = get_constant(constants, i);
        printf("[%d] %f %f %f %f\n", i, constant->x, constant->y, constant->z, constant->w);
    }
}



static size_t check_symbols(const char *str, size_t len, size_t offset) {
    static struct { const char *str; size_t len; } SYMBOLS[] = {
#define CC_EXT_OP(name, args, precedence, symbol) { symbol, sizeof(symbol) - 1 },
#include "gfx_cc_ext_op.inl"
#undef CC_EXT_OP
        { "(", 1 },
        { ")", 1 },
        { ",", 1 },
    };

    size_t max_size = 0;
    for (int i = 0; i < ARRAY_COUNT(SYMBOLS); ++i) {
        size_t symb_len = SYMBOLS[i].len;
        if (max_size < symb_len && offset + symb_len <= len && strnicmp(str + offset, SYMBOLS[i].str, SYMBOLS[i].len) == 0) {
            max_size = symb_len;
        }
    }
    return max_size;
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
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];

        // ignore whitespaces and others
        if (iscntrl(c) || isspace(c)) {
            push_curr_token();
            continue;
        }

        // symbols
        size_t symb_max_size = check_symbols(str, len, i);
        if (symb_max_size > 0 && (symb_max_size > 1 || c != '.' || !is_number)) {
            push_curr_token();
            push_token(tokens, str + i, i, symb_max_size);
            i += (symb_max_size - 1);
            is_dot_arg = (symb_max_size == 1 && c == '.');
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

    // push remaining token
    push_curr_token();

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

        // manually checked symbols
        // includes unary plus and minus, parentheses and comma ops

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

        // symbols
#define CC_EXT_OP(name, args, precedence, symbol) token_to_op(symbol, CC_EXT_OP_##name, precedence == 1);
#include "gfx_cc_ext_op.inl"
#undef CC_EXT_OP

        // functions
#define CC_EXT_FUNC(name, args) token_to_function(#name, CC_EXT_FUNC_##name);
#include "gfx_cc_ext_func.inl"
#undef CC_EXT_FUNC

        // values
#define CC_EXT_VAL(name) token_to_value(#name, CC_EXT_VAL_##name);
#include "gfx_cc_ext_val.inl"
#undef CC_EXT_VAL

        // other values
        token_to_value("0.0", CC_EXT_VAL_0);
        token_to_value("1.0", CC_EXT_VAL_1);
        token_to_value("TEXEL0_ALPHA", CC_EXT_VAL_TEXEL0A);
        token_to_value("TEXEL1_ALPHA", CC_EXT_VAL_TEXEL1A);
        token_to_value("PRIMITIVE", CC_EXT_VAL_PRIM);
        token_to_value("PRIMITIVE_ALPHA", CC_EXT_VAL_PRIMA);
        token_to_value("SHADE_ALPHA", CC_EXT_VAL_SHADEA);
        token_to_value("ENVIRONMENT", CC_EXT_VAL_ENV);
        token_to_value("ENV_ALPHA", CC_EXT_VAL_ENVA);
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

    prefix = infix_to_prefix(ops, sErrorMsg, sizeof(sErrorMsg), str);
    if (!prefix) {
        goto cleanup;
    }

    print_ops(prefix, constants);
    printf("\n");

    // validate


    // optimize


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
    "ENV * (-TEXEL0 + TEXEL1.a).gb1 + (-0.5 >= 0.2) * 99 + vec(9, 10, 11, 12) - min(0.5, max(lerp(PRIM + ENVA * 0.75, SHADE, PRIMA).00ra.gbra, 0)) + 2 * 1 - 1.5 ^ (3 + 1)";


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

