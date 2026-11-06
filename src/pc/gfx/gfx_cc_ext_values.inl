//
// Extended CC values
// ------------------
// The color sources.
// Macro params are: (name)
//
// Note: The order does not follow the CCS constants!
// COMBINED sources are also missing, since there is no cycle 2 in extended combiners.
//

CC_EXT_VALUE(0)
CC_EXT_VALUE(1)

// Inputs
CC_EXT_VALUE(PRIM)
CC_EXT_VALUE(PRIMA)
CC_EXT_VALUE(SHADE)
CC_EXT_VALUE(SHADEA)
CC_EXT_VALUE(ENV)
CC_EXT_VALUE(ENVA)
CC_EXT_VALUE(LOD)
CC_EXT_VALUE(NOISE)
CC_EXT_VALUE(AMBIENT)

// Lights
// #define CC_EXT_VALUE_LIGHT(n) \
//     CC_EXT_VALUE(LIGHT##n) \
//     CC_EXT_VALUE(LIGHT##n##I)
// REPEAT_FROM_0(CC_EXT_VALUE_LIGHT, MAX_LIGHTS)
// #undef CC_EXT_VALUE_LIGHT

// Textures
#define CC_EXT_VALUE_TEXEL(n) \
    CC_EXT_VALUE(TEXEL##n) \
    CC_EXT_VALUE(TEXEL##n##A)
REPEAT_FROM_0(CC_EXT_VALUE_TEXEL, MAX_TEXTURES)
#undef CC_EXT_VALUE_TEXEL
