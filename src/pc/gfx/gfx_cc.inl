//
// Keep everything in this file as C macros!
// All constants here are also included in the ubershader,
// and GLSL doesn't have enum types.
//

//
// Color combiner structure
//

#define CC_COMPONENTS   4 // ABCD
#define CC_CHANNELS     2 // RGB/A
#define CC_CYCLES       2 // Cycles
#define CC_MODES        (CC_CYCLES * CC_CHANNELS)
#define CC_CMD_LENGTH   (CC_MODES * CC_COMPONENTS)

//
// Textures
//

#define MAX_TEXTURES    2

//
// Shader inputs
//

#define SHADER_MAX_INPUTS   8

#define SHADER_0            0
#define SHADER_TEXEL0       1
#define SHADER_TEXEL0A      2
#define SHADER_TEXEL1       3
#define SHADER_TEXEL1A      4
#define SHADER_1            5
#define SHADER_COMBINED     6
#define SHADER_COMBINEDA    7
#define SHADER_NOISE        8
#define SHADER_INPUTS       9 // must be last
#define SHADER_ENUM_MAX     (SHADER_INPUTS + SHADER_MAX_INPUTS)

//
// CC features flags
//

#define CCF_FLAG_USE_ALPHA          0   // 1 bit
#define CCF_FLAG_USE_FOG            1   // 1 bit
#define CCF_FLAG_TEX_EDGE           2   // 1 bit
#define CCF_FLAG_USE_DITHER         3   // 1 bit
#define CCF_FLAG_USE_2CYCLE         4   // 1 bit
#define CCF_FLAG_LIGHT_MAP          5   // 1 bit
#define CCF_FLAG_WORLD_GEOMETRY     6   // 1 bit

#define CCF_FLAG_COLOR_ALPHA_SAME   8                                         // 2 bits
#define CCF_FLAG_DO_SINGLE          (CCF_FLAG_COLOR_ALPHA_SAME + CC_CYCLES)   // 4 bits
#define CCF_FLAG_DO_MULTIPLY        (CCF_FLAG_DO_SINGLE        + CC_MODES)    // 4 bits
#define CCF_FLAG_DO_MIX             (CCF_FLAG_DO_MULTIPLY      + CC_MODES)    // 4 bits
#define CCF_FLAG_DO_NOISE           (CCF_FLAG_DO_MIX           + CC_MODES)    // 1 bit
#define CCF_FLAG_USED_TEXTURE       (CCF_FLAG_DO_NOISE         + 1)           // MAX_TEXTURES bits
