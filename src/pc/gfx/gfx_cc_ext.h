#ifndef GFX_CC_EXT_H
#define GFX_CC_EXT_H

#include <stdint.h>

#define CC_EXT_FLAG_OPERATOR            0x80
#define CC_EXT_OPERATOR_ARGS_SHIFT      5
#define CC_EXT_OPERATOR_ARGS_WIDTH      2
#define CC_EXT_OPERATOR_ARGS_BITS       ((1 << CC_EXT_OPERATOR_ARGS_WIDTH) - 1)
#define CC_EXT_OPERATOR_ARGS(op)        ((((op) >> CC_EXT_OPERATOR_ARGS_SHIFT) & CC_EXT_OPERATOR_ARGS_BITS) + 1)

#define CC_EXT_DOT_PARAM_0      0
#define CC_EXT_DOT_PARAM_1      1
#define CC_EXT_DOT_PARAM_R      2
#define CC_EXT_DOT_PARAM_G      3
#define CC_EXT_DOT_PARAM_B      4
#define CC_EXT_DOT_PARAM_A      5


enum {
#define CC_EXT_VALUE(name) CC_EXT_ID_##name,
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
};

enum {
#define CC_EXT_OPERATOR_1(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR_2(name)
#define CC_EXT_OPERATOR_3(name)
#define CC_EXT_OPERATOR_4(name)
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION_2(name)
#define CC_EXT_FUNCTION_3(name)
#define CC_EXT_FUNCTION_4(name)
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};
enum {
#define CC_EXT_OPERATOR_1(name)
#define CC_EXT_OPERATOR_2(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR_3(name)
#define CC_EXT_OPERATOR_4(name)
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name)
#define CC_EXT_FUNCTION_2(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION_3(name)
#define CC_EXT_FUNCTION_4(name)
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};
enum {
#define CC_EXT_OPERATOR_1(name)
#define CC_EXT_OPERATOR_2(name)
#define CC_EXT_OPERATOR_3(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR_4(name)
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name)
#define CC_EXT_FUNCTION_2(name)
#define CC_EXT_FUNCTION_3(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION_4(name)
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};
enum {
#define CC_EXT_OPERATOR_1(name)
#define CC_EXT_OPERATOR_2(name)
#define CC_EXT_OPERATOR_3(name)
#define CC_EXT_OPERATOR_4(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name)
#define CC_EXT_FUNCTION_2(name)
#define CC_EXT_FUNCTION_3(name)
#define CC_EXT_FUNCTION_4(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};



enum {

// Values
#define CC_EXT_VALUE(name) CC_EXT_##name = CC_EXT_ID_##name,
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
    CC_EXT_CONST_FIRST,
    CC_EXT_CONST_LAST = (CC_EXT_FLAG_OPERATOR - 1),

// Operators
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_##name = (CC_EXT_ID_##name | ((args - 1) << CC_EXT_OPERATOR_ARGS_SHIFT) | CC_EXT_FLAG_OPERATOR),
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, args) CC_EXT_##name = (CC_EXT_ID_##name | ((args - 1) << CC_EXT_OPERATOR_ARGS_SHIFT) | CC_EXT_FLAG_OPERATOR),
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

// Special ops for parsing
    CC_EXT_OPEN  = 0xFD,
    CC_EXT_CLOSE = 0xFE,
    CC_EXT_COMMA = 0xFF,

// Special op for validating
    CC_EXT_RESULT = 0xFF,
};


#define CC_EXT_MAX_OPS           128
#define CC_EXT_MAX_CONSTANTS     (CC_EXT_CONST_LAST - CC_EXT_CONST_FIRST + 1)
#define CC_EXT_STACK_SIZE        64


struct ExtendedCombiner {
    uint8_t ops[CC_EXT_MAX_OPS];
    uint32_t num_ops;
    float constants[CC_EXT_MAX_CONSTANTS][4];
    uint32_t num_constants;
};


#ifdef DEFINE_CC_EXT_TYPES
#include "gfx_cc_ext_types.h"
#endif

struct GrowingArray;

void gfx_cc_ext_optimize(struct GrowingArray *ops, struct GrowingArray *constants);



#endif
