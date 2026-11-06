#include <stdio.h>

#include "gfx_cc_ext.h"
#include "gfx_cc_ext_types.h"

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

// Registers
"#define CC_EXT_REGISTER_FIRST %3d\n"
"#define CC_EXT_REGISTER_LAST %3d\n"
"\n"

// Constants
"#define CC_EXT_CONST_FIRST %3d\n"
"#define CC_EXT_CONST_LAST %3d\n"
"\n"

//
// Stack
//

#define DEFINE_ARG(n) "#define ARG" #n " stack[top - " #n "]\n"
REPEAT(DEFINE_ARG, CC_EXT_MAX_ARGS)
"#define STACK_PUSH(args, operation) { vec4 res = operation; stack[top - args] = res; top -= (args - 1); }\n"
"\n"

//
// Built-in definitions
//

"#define v_0             vec4(0.0, 0.0, 0.0, 0.0)\n"
"#define v_1             vec4(1.0, 1.0, 1.0, 1.0)\n"

// Inputs
"#define v_PRIM          vPrimitive\n"
"#define v_PRIMA         vPrimitive.aaaa\n"
"#define v_SHADE         vShade\n"
"#define v_SHADEA        vShade.aaaa\n"
"#define v_ENV           vEnvironment\n"
"#define v_ENVA          vEnvironment.aaaa\n"
"#define v_LOD           vLod\n"
"#define v_NOISE         noise\n"
"#define v_AMBIENT       vAmbient\n"

// Lights
// #define CC_EXT_VALUE_LIGHT(n) \
// "#define v_LIGHT" #n "        vec4(vLights[" #n "].rgb, 1.0)\n" \
// "#define v_LIGHT" #n "I       vLights[" #n "].aaaa\n"
// REPEAT_FROM_0(CC_EXT_VALUE_LIGHT, MAX_ACTIVE_LIGHTS)
// #undef CC_EXT_VALUE_LIGHT

// Textures
#define CC_EXT_VALUE_TEXEL(n) \
"#define v_TEXEL" #n "        texColors[" #n "]\n" \
"#define v_TEXEL" #n "A       texColors[" #n "].aaaa\n"
REPEAT_FROM_0(CC_EXT_VALUE_TEXEL, MAX_TEXTURES)
#undef CC_EXT_VALUE_TEXEL

"\n"




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

"        default: \\\n"

// Registers
"            if (op >= CC_EXT_REGISTER_FIRST && op <= CC_EXT_REGISTER_LAST) { \\\n"
"                STACK_PUSH(0, registers[op - CC_EXT_REGISTER_FIRST]); \\\n"
"                break; \\\n"
"            } \\\n"

// Constants
"            if (op >= CC_EXT_CONST_FIRST && op <= CC_EXT_CONST_LAST) { \\\n"
"                STACK_PUSH(0, uExtCC[alpha].constants[op - CC_EXT_CONST_FIRST]); \\\n"
"                break; \\\n"
"            } \\\n"

"            break; \\\n"
"    } \\\n"
"} \n"
"\n"
    ;

    // Enums must be resolved at runtime
    static char sExtCombinerShaderSource[sizeof(sExtCombinerShaderSourceTemplate)];
    snprintf(sExtCombinerShaderSource, sizeof(sExtCombinerShaderSource), sExtCombinerShaderSourceTemplate

// Values
#define CC_EXT_VALUE(name) ,CC_EXT_##name
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE

// Operators
#define CC_EXT_OPERATOR(name, ...) ,CC_EXT_##name
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, ...) ,CC_EXT_##name
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

// Registers
    ,CC_EXT_REGISTER_FIRST
    ,CC_EXT_REGISTER_LAST

// Constants
    ,CC_EXT_CONST_FIRST
    ,CC_EXT_CONST_LAST
    );

    return sExtCombinerShaderSource;
}
