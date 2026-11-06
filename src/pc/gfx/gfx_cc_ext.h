#ifndef GFX_CC_EXT_H
#define GFX_CC_EXT_H

#include <stdint.h>

#include "gfx_cc.inl"
#include "gfx_cc_ext.inl"
#include "macros.h"

enum {

// Values
#define CC_EXT_VALUE(name) CC_EXT_##name,
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
    CC_EXT_VALUE_COUNT,
    CC_EXT_VALUE_LAST = CC_EXT_VALUE_COUNT - 1,

// Operators
#define CC_EXT_OPERATOR(name, ...) CC_EXT_##name,
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, ...) CC_EXT_##name,
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

// Registers
    CC_EXT_REGISTER_FIRST,
    CC_EXT_REGISTER_LAST = (CC_EXT_REGISTER_FIRST + CC_EXT_MAX_REGISTERS - 1),

// Constants
    CC_EXT_CONST_FIRST,
    CC_EXT_CONST_LAST = (CC_EXT_CONST_FIRST + CC_EXT_MAX_CONSTANTS - 1),

// Special ops for parsing
    CC_EXT_SPECIAL = 0xFD,
    CC_EXT_OPEN    = (CC_EXT_SPECIAL + 0),
    CC_EXT_CLOSE   = (CC_EXT_SPECIAL + 1),
    CC_EXT_COMMA   = (CC_EXT_SPECIAL + 2),

// Special op for validating
    CC_EXT_RESULT = 0xFF,
};

//
// Extended combiner
//

struct ExtendedCombiner {
    uint8_t ops[CC_EXT_MAX_OPS];
    uint32_t num_ops;
    float constants[CC_EXT_MAX_CONSTANTS][4];
    uint32_t num_constants;
};

const char *gfx_cc_ext_get_shader_source();
const struct ExtendedCombiner *gfx_cc_ext_get_combiner(const char *str, const char **error_msg);
void gfx_cc_ext_clear_combiner_cache();

#endif
