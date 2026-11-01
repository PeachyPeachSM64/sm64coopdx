#include <stdio.h>

#include "macros.h"
#include "gfx_cc_ext.h"

//
// Shader source for extended combiners
// Defines constants and functions for use in the ubershader.
//

const char *gfx_cc_ext_get_shader_source() {
    static const char sExtCombinerShaderSourceTemplate[] = ""

//
// Enum definition
//

// Values
#define CC_EXT_VALUE(name) "#define CC_EXT_" #name " %3d\n"
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
"#define CC_EXT_CONST_FIRST %3d\n"
"#define CC_EXT_CONST_LAST %3d\n"
"\n"

// Operators
#define CC_EXT_OPERATOR(name, ...) "#define CC_EXT_" #name " %3d\n"
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR
"\n"

// Functions
#define CC_EXT_FUNCTION(name, ...) "#define CC_EXT_" #name " %3d\n"
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION
"\n"

//
// Stack
//

#define DEFINE_ARG(n) "#define ARG" #n " stack[top - " #n "]\n"
REPEAT(DEFINE_ARG, 4) // CC_EXT_MAX_ARGS
"#define STACK_PUSH(args, operation) { vec4 res = (operation); stack[top - args] = res; top -= (args - 1); }\n"
"\n"

//
// 
//


//
// Function to process an op
//

"#define cc_ext_process_op(op) { \\\n"
"    switch (op) { \\\n"

// Values
#define CC_EXT_VALUE(name) "        case CC_EXT_" #name ": STACK_PUSH(0, v_" #name "); break; \\\n"
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE

// Operators
#define CC_EXT_OPERATOR(name, args, ...) "        case CC_EXT_" #name ": STACK_PUSH(" #args ", o_" #name "); break; \\\n"
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, args) "        case CC_EXT_" #name ": STACK_PUSH(" #args ", f_" #name "); break; \\\n"
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

// Constants
"        default: STACK_PUSH(0, uExtCC[alpha].constants[op - CC_EXT_CONST_FIRST]); break; \\\n"
"    } \\\n"
"} \\\n"
"\n"
    ;

    // Enums must be resolved at runtime
    static char sExtCombinerShaderSource[sizeof(sExtCombinerShaderSourceTemplate)];
    snprintf(sExtCombinerShaderSource, sizeof(sExtCombinerShaderSource), sExtCombinerShaderSourceTemplate

// Values
#define CC_EXT_VALUE(name) ,CC_EXT_##name
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
    ,CC_EXT_CONST_FIRST
    ,CC_EXT_CONST_LAST

// Operators
#define CC_EXT_OPERATOR(name, ...) ,CC_EXT_##name
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, ...) ,CC_EXT_##name
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION
    );

    return sExtCombinerShaderSource;
}
