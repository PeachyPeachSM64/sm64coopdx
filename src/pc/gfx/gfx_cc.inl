//
// Keep everything in this file as C macros!
// All constants here are also included in the ubershader,
// and GLSL doesn't have enum types.
//

#ifndef SHADER_CODE
#pragma once
#include "macros.h"
#endif

//
// Color combiner structure
//

#define CC_COMPONENTS       4  // ABCD
#define CC_CYCLES           2  // Cycles
#define MAX_ACTIVE_LIGHTS   8  // must be a multiple of 4
#define MAX_TEXTURES        2  // must be a multiple of 2

//
// CC sources
//

#define CCS_0               0
#define CCS_1               1
#define CCS_COMBINED        2
#define CCS_COMBINEDA       3
#define CCS_PRIM            4
#define CCS_PRIMA           5
#define CCS_SHADE           6
#define CCS_SHADEA          7
#define CCS_ENV             8
#define CCS_ENVA            9
#define CCS_LOD             10
#define CCS_NOISE           11
#define CCS_AMBIENT         12

//
// Lights
//

#define CCS_LIGHT_FIRST     (CCS_AMBIENT + 1)
#define CCS_LIGHT_LAST      (CCS_LIGHT_FIRST + 2 * MAX_ACTIVE_LIGHTS - 1)

// defines CCS_LIGHT0, CCS_LIGHT0I, etc... (for gsDPSetCombine... only)
#ifndef SHADER_CODE
#define CCS_LIGHT_(n) \
static const uint8_t CCS_LIGHT##n    = CCS_LIGHT_FIRST + 2 * n; \
static const uint8_t CCS_LIGHT##n##I = CCS_LIGHT_FIRST + 2 * n + 1;
REPEAT_FROM_0(CCS_LIGHT_, MAX_ACTIVE_LIGHTS)
#undef CCS_LIGHT_
#endif

//
// Textures
//

#define CCS_TEXEL_FIRST     (CCS_LIGHT_LAST + 1)
#define CCS_TEXEL_LAST      (CCS_TEXEL_FIRST + 2 * MAX_TEXTURES - 1)

// defines CCS_TEXEL0, CCS_TEXEL0A, etc... (for gsDPSetCombine... only)
#ifndef SHADER_CODE
#define CCS_TEXEL_(n) \
static const uint8_t CCS_TEXEL##n    = CCS_TEXEL_FIRST + 2 * n; \
static const uint8_t CCS_TEXEL##n##A = CCS_TEXEL_FIRST + 2 * n + 1;
REPEAT_FROM_0(CCS_TEXEL_, MAX_TEXTURES)
#undef CCS_TEXEL_
#endif

// aliases (for gsDPSetCombine... only)
#ifndef SHADER_CODE
#define CCS_COUNT               (CCS_TEXEL_LAST + 1)
#define CCS_TEXEL0_ALPHA        CCS_TEXEL0A
#define CCS_TEXEL1_ALPHA        CCS_TEXEL1A
#define CCS_COMBINED_ALPHA      CCS_COMBINEDA
#define CCS_PRIMITIVE           CCS_PRIM
#define CCS_PRIMITIVE_ALPHA     CCS_PRIMA
#define CCS_SHADE_ALPHA         CCS_SHADEA
#define CCS_ENVIRONMENT         CCS_ENV
#define CCS_ENV_ALPHA           CCS_ENVA
#define CCS_LOD_FRACTION        CCS_LOD
#endif

//
// CC features
//

#define CCF_USE_ALPHA               0
#define CCF_USE_FOG                 1
#define CCF_TEX_EDGE                2
#define CCF_USE_DITHER              3
#define CCF_USE_2CYCLE              4
#define CCF_LIGHT_MAP               5
#define CCF_WORLD_GEOMETRY          6
#define CCF_USE_NOISE               7
#define CCF_USE_EXT_COLOR           8
#define CCF_USE_EXT_ALPHA           9
#define CCF_EXT_COLOR_ALPHA_SAME    10
#define CCF_USED_TEXTURE            11 // must be last

#define USED_TEXTURE(i)  (1 << (i))

#define UNPACK(v, s, w) \
    ((int(v) >> (s)) & ((1 << (w)) - 1))
