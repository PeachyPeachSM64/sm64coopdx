//
// Keep everything in this file as C macros!
// All constants here are also included in the ubershader,
// and GLSL doesn't have enum types.
// `SHADER_CODE` is defined only inside shaders.
//

#if defined(SHADER_CODE) || defined(DEFINE_CC_EXT_CONSTANTS)

#define CC_EXT_MAX_OPS          128  // includes terminator
#define CC_EXT_MAX_CONSTANTS    32
#define CC_EXT_MAX_REGISTERS    8
#define CC_EXT_STACK_SIZE       16

#define CC_EXT_DOT_PARAM_0      0
#define CC_EXT_DOT_PARAM_1      1
#define CC_EXT_DOT_PARAM_R      2
#define CC_EXT_DOT_PARAM_G      3
#define CC_EXT_DOT_PARAM_B      4
#define CC_EXT_DOT_PARAM_A      5

#endif

//
// Extended CC values
// ------------------
// The color sources.
// Macro params are: (name)
//

#if defined(SHADER_CODE)
#define CC_EXT_VALUE(name) static const int CC_EXT_##name = CCS_##name;
#elif defined(DEFINE_CC_EXT_CONSTANTS)
#define CC_EXT_VALUE(name) CC_EXT_##name = CCS_##name,
#elif !defined(CC_EXT_VALUE)
#define CC_EXT_VALUE(...)
#endif

CC_EXT_VALUE(0)
CC_EXT_VALUE(1)
CC_EXT_VALUE(COMBINED)
CC_EXT_VALUE(COMBINEDA)
CC_EXT_VALUE(PRIM)
CC_EXT_VALUE(PRIMA)
CC_EXT_VALUE(SHADE)
CC_EXT_VALUE(SHADEA)
CC_EXT_VALUE(ENV)
CC_EXT_VALUE(ENVA)
CC_EXT_VALUE(LOD)
CC_EXT_VALUE(NOISE)
CC_EXT_VALUE(AMBIENT)

//
// Lights
//

#if defined(SHADER_CODE) || defined(DEFINE_CC_EXT_CONSTANTS)
#define CC_EXT_LIGHT_FIRST  CCS_LIGHT_FIRST
#define CC_EXT_LIGHT_LAST   CCS_LIGHT_LAST
#endif

#ifndef SHADER_CODE
#define CC_EXT_VALUE_LIGHT(n) \
CC_EXT_VALUE(LIGHT##n) \
CC_EXT_VALUE(LIGHT##n##I)
REPEAT_FROM_0(CC_EXT_VALUE_LIGHT, MAX_ACTIVE_LIGHTS)
#undef CC_EXT_VALUE_LIGHT
#endif

//
// Textures
//

#if defined(SHADER_CODE) || defined(DEFINE_CC_EXT_CONSTANTS)
#define CC_EXT_TEXEL_FIRST  CCS_TEXEL_FIRST
#define CC_EXT_TEXEL_LAST   CCS_TEXEL_LAST
#endif

#ifndef SHADER_CODE
#define CC_EXT_VALUE_TEXEL(n) \
CC_EXT_VALUE(TEXEL##n) \
CC_EXT_VALUE(TEXEL##n##A)
REPEAT_FROM_0(CC_EXT_VALUE_TEXEL, MAX_TEXTURES)
#undef CC_EXT_VALUE_TEXEL
#endif

//
// Registers
//

#ifndef CC_EXT_REGISTER_FIRST
#define CC_EXT_REGISTER_FIRST   (CC_EXT_TEXEL_LAST + 1)
#define CC_EXT_REGISTER_LAST    (CC_EXT_REGISTER_FIRST + CC_EXT_MAX_REGISTERS - 1)
#endif

//
// Constants
//

#ifndef CC_EXT_CONST_FIRST
#define CC_EXT_CONST_FIRST      (CC_EXT_REGISTER_LAST + 1)
#define CC_EXT_CONST_LAST       (CC_EXT_CONST_FIRST + CC_EXT_MAX_CONSTANTS - 1)
#endif

//
// Extended CC operators
// ---------------------
// Unary and binary operators.
// Macro params are: (id, name, args, precedence, symbol)
//

#if defined(SHADER_CODE)
#define CC_EXT_OPERATOR(id, name, args, precedence, symbol) static const int CC_EXT_##name = CC_EXT_CONST_LAST + 1 + (id);
#elif defined(DEFINE_CC_EXT_CONSTANTS)
#define CC_EXT_OPERATOR(id, name, args, precedence, symbol) CC_EXT_##name = CC_EXT_CONST_LAST + 1 + (id),
#elif !defined(CC_EXT_OPERATOR)
#define CC_EXT_OPERATOR(...)
#endif

CC_EXT_OPERATOR( 0, DOT, 2, 0, ".")
CC_EXT_OPERATOR( 1, NEG, 1, 1, "-")
CC_EXT_OPERATOR( 2, NOT, 1, 1, "!")
CC_EXT_OPERATOR( 3, POW, 2, 2, "^")
CC_EXT_OPERATOR( 4, MUL, 2, 3, "*")
CC_EXT_OPERATOR( 5, DIV, 2, 3, "/")
CC_EXT_OPERATOR( 6, MOD, 2, 3, "%")
CC_EXT_OPERATOR( 7, ADD, 2, 4, "+")
CC_EXT_OPERATOR( 8, SUB, 2, 4, "-")
CC_EXT_OPERATOR( 9, LT,  2, 5, "<")
CC_EXT_OPERATOR(10, LEQ, 2, 5, "<=")
CC_EXT_OPERATOR(11, GT,  2, 5, ">")
CC_EXT_OPERATOR(12, GEQ, 2, 5, ">=")
CC_EXT_OPERATOR(13, EQ,  2, 5, "==")
CC_EXT_OPERATOR(14, NEQ, 2, 5, "!=")
CC_EXT_OPERATOR(15, AND, 2, 6, "&&")
CC_EXT_OPERATOR(16, OR,  2, 6, "||")
CC_EXT_OPERATOR(17, SET, 2, 7, "=")
CC_EXT_OPERATOR(18, END, 0, 0, ";") // must be last

//
// Extended CC functions
// ---------------------
// Functions with fixed number of arguments.
// Macro params are: (id, name, args)
//

#if defined(SHADER_CODE)
#define CC_EXT_FUNCTION(id, name, args) static const int CC_EXT_##name = CC_EXT_END + 1 + (id);
#elif defined(DEFINE_CC_EXT_CONSTANTS)
#define CC_EXT_FUNCTION(id, name, args) CC_EXT_##name = CC_EXT_END + 1 + (id),
#elif !defined(CC_EXT_FUNCTION)
#define CC_EXT_FUNCTION(...)
#endif

CC_EXT_FUNCTION( 0, VEC, 4)
CC_EXT_FUNCTION( 1, LERP, 3)
CC_EXT_FUNCTION( 2, INVLERP, 3)
CC_EXT_FUNCTION( 3, REMAP, 5)
CC_EXT_FUNCTION( 4, IF, 3)
CC_EXT_FUNCTION( 5, ABS, 1)
CC_EXT_FUNCTION( 6, MIN, 2)
CC_EXT_FUNCTION( 7, MAX, 2)
CC_EXT_FUNCTION( 8, CLAMP, 3)
CC_EXT_FUNCTION( 9, SATURATE, 1)
CC_EXT_FUNCTION(10, COS, 1)
CC_EXT_FUNCTION(11, SIN, 1)
CC_EXT_FUNCTION(12, SQRT, 1)
CC_EXT_FUNCTION(13, RSQRT, 1)
CC_EXT_FUNCTION(14, ROUND, 1)
CC_EXT_FUNCTION(15, FLOOR, 1)
CC_EXT_FUNCTION(16, CEIL, 1)


#undef CC_EXT_VALUE
#undef CC_EXT_OPERATOR
#undef CC_EXT_FUNCTION
