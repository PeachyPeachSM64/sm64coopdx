#ifndef GFX_CC_EXT_H
#define GFX_CC_EXT_H

#include <stdint.h>

#include "gfx_cc.inl"

enum {
#define DEFINE_CC_EXT_CONSTANTS
#include "gfx_cc_ext.inl"
#undef DEFINE_CC_EXT_CONSTANTS
    CC_EXT_COUNT,

// Terminator, end of expression
    CC_EXT_TERM    = 0xFF,  

// Special ops for parsing
    CC_EXT_SPECIAL = 0xFD,
    CC_EXT_OPEN    = (CC_EXT_SPECIAL + 0),
    CC_EXT_CLOSE   = (CC_EXT_SPECIAL + 1),
    CC_EXT_COMMA   = (CC_EXT_SPECIAL + 2),

// Special op for validating
    CC_EXT_RESULT  = 0xFF,
};

//
// Extended combiner
//

struct ExtendedCombiner {
    uint8_t ops[CC_EXT_MAX_OPS];
    float constants[CC_EXT_MAX_CONSTANTS][4];
};

struct ColorCombiner;

const char *gfx_cc_ext_get_shader_source();
const struct ExtendedCombiner *gfx_cc_ext_get_combiner(const char *str, const char **error_msg);
const struct ExtendedCombiner *gfx_cc_ext_get_combiner_from_cc(const struct ColorCombiner *cc, bool alpha);
void gfx_cc_ext_clear_combiner_cache(bool force);

#endif
