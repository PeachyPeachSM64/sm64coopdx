#ifndef GFX_CC_SOURCES_H
#define GFX_CC_SOURCES_H

#include "macros.h"

#define CC_COMPONENTS       4  // ABCD
#define CC_CYCLES           2  // Cycles
#define MAX_TEXTURES        2  // must be a multiple of 2
#define MAX_ACTIVE_LIGHTS   8  // must be a multiple of 4

//
// CC sources
//

enum {
    CCS_0,
    CCS_1,
    CCS_COMBINED,
    CCS_COMBINEDA,
    CCS_PRIM,
    CCS_PRIMA,
    CCS_SHADE,
    CCS_SHADEA,
    CCS_ENV,
    CCS_ENVA,
    CCS_LOD,
    CCS_NOISE,
    CCS_AMBIENT,

// Lights
#define CC_EXT_VALUE_LIGHT(n) \
    CCS_LIGHT##n, \
    CCS_LIGHT##n##I,
REPEAT_FROM_0(CC_EXT_VALUE_LIGHT, MAX_ACTIVE_LIGHTS)
#undef CC_EXT_VALUE_LIGHT

// Textures
#define CC_EXT_VALUE_TEXEL(n) \
    CCS_TEXEL##n, \
    CCS_TEXEL##n##A,
REPEAT_FROM_0(CC_EXT_VALUE_TEXEL, MAX_TEXTURES)
#undef CC_EXT_VALUE_TEXEL

    CCS_COUNT,

// Aliases
    CCS_TEXEL0_ALPHA = CCS_TEXEL0A,
    CCS_TEXEL1_ALPHA = CCS_TEXEL1A,
    CCS_COMBINED_ALPHA = CCS_COMBINEDA,
    CCS_PRIMITIVE = CCS_PRIM,
    CCS_PRIMITIVE_ALPHA = CCS_PRIMA,
    CCS_SHADE_ALPHA = CCS_SHADEA,
    CCS_ENVIRONMENT = CCS_ENV,
    CCS_ENV_ALPHA = CCS_ENVA,
    CCS_LOD_FRACTION = CCS_LOD,
};

//
// CC features
//

enum {
    CCF_USE_ALPHA,
    CCF_USE_FOG,
    CCF_TEX_EDGE,
    CCF_USE_DITHER,
    CCF_USE_2CYCLE,
    CCF_LIGHT_MAP,
    CCF_WORLD_GEOMETRY,
    CCF_USE_NOISE,
    CCF_USE_EXT_COLOR,
    CCF_USE_EXT_ALPHA,
    CCF_EXT_COLOR_ALPHA_SAME,
    CCF_USED_TEXTURE,
};

#define USED_TEXTURE(i) (1 << (i))

#endif
