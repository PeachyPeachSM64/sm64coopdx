#include <stdio.h>

#include "gfx_cc.h"
#include "gfx_pc.h"

//
// Shader source for the color combiner
// Defines constants and functions for use in the ubershader.
//

const char *gfx_cc_get_shader_source() {
    static const char sShaderSourceTemplate[] = ""

//
// Constants
//

"#define CC_COMPONENTS %3d\n"
"#define CC_CYCLES %3d\n"
"#define MAX_TEXTURES %3d\n"
"#define MAX_ACTIVE_LIGHTS %3d\n"

"\n"

//
// CC sources
//

"#define CCS_0 %3d\n"
"#define CCS_1 %3d\n"
"#define CCS_COMBINED %3d\n"
"#define CCS_COMBINEDA %3d\n"
"#define CCS_PRIM %3d\n"
"#define CCS_PRIMA %3d\n"
"#define CCS_SHADE %3d\n"
"#define CCS_SHADEA %3d\n"
"#define CCS_ENV %3d\n"
"#define CCS_ENVA %3d\n"
"#define CCS_LOD %3d\n"
"#define CCS_NOISE %3d\n"
"#define CCS_AMBIENT %3d\n"

// Lights
#define CC_EXT_VALUE_LIGHT(n) \
"#define CCS_LIGHT" #n " %3d\n" \
"#define CCS_LIGHT" #n "I %3d\n"
REPEAT_FROM_0(CC_EXT_VALUE_LIGHT, MAX_ACTIVE_LIGHTS)
#undef CC_EXT_VALUE_LIGHT

// Textures
#define CC_EXT_VALUE_TEXEL(n) \
"#define CCS_TEXEL" #n " %3d\n" \
"#define CCS_TEXEL" #n "A %3d\n"
REPEAT_FROM_0(CC_EXT_VALUE_TEXEL, MAX_TEXTURES)
#undef CC_EXT_VALUE_TEXEL

"#define CCS_COUNT %3d\n"

"\n"

//
// CC features
//

"#define CCF_USE_ALPHA %3d\n"
"#define CCF_USE_FOG %3d\n"
"#define CCF_TEX_EDGE %3d\n"
"#define CCF_USE_DITHER %3d\n"
"#define CCF_USE_2CYCLE %3d\n"
"#define CCF_LIGHT_MAP %3d\n"
"#define CCF_WORLD_GEOMETRY %3d\n"
"#define CCF_USE_NOISE %3d\n"
"#define CCF_USE_EXT_COLOR %3d\n"
"#define CCF_USE_EXT_ALPHA %3d\n"
"#define CCF_EXT_COLOR_ALPHA_SAME %3d\n"
"#define CCF_USED_TEXTURE %3d\n"

"\n"

//
// Other
//

// Global shader effect flags
"#define SHADER_FLAG_HUE %3d\n"
"#define SHADER_FLAG_SATURATION %3d\n"
"#define SHADER_FLAG_BRIGHTNESS %3d\n"
"#define SHADER_FLAG_CONTRAST %3d\n"
"#define SHADER_FLAG_EXPOSURE %3d\n"
"#define SHADER_FLAG_DITHERING %3d\n"
"#define SHADER_FLAG_POSTERIZATION %3d\n"
"#define SHADER_FLAG_SCANLINES %3d\n"
"#define SHADER_FLAG_MAX %3d\n"

// Texture filtering
"#define FILTER_THREE_POINT %3d\n"

"\n"

//
// Utilities
//

// Texture bit
"#define USED_TEXTURE(i) (1 << (i))\n"

// Color unpacking
"#define UNPACK(v, s, w) ((int(v) >> (s)) & ((1 << (w)) - 1))\n"

"\n";

    // Constants and enums must be resolved at runtime
    static char sShaderSource[sizeof(sShaderSourceTemplate)];
    snprintf(sShaderSource, sizeof(sShaderSource), sShaderSourceTemplate,

        // Constants
        CC_COMPONENTS,
        CC_CYCLES,
        MAX_TEXTURES,
        MAX_ACTIVE_LIGHTS,

        // CC sources
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

        // CC features
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

        // Global shader effect flags
        SHADER_FLAG_HUE,
        SHADER_FLAG_SATURATION,
        SHADER_FLAG_BRIGHTNESS,
        SHADER_FLAG_CONTRAST,
        SHADER_FLAG_EXPOSURE,
        SHADER_FLAG_DITHERING,
        SHADER_FLAG_POSTERIZATION,
        SHADER_FLAG_SCANLINES,
        SHADER_FLAG_MAX,

        // Texture filtering
        2
    );

    return sShaderSource;
}
