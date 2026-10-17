//
// Keep everything in this file as C macros!
// All constants here are also included in the ubershader,
// and GLSL doesn't have enum types.
//

//
// Color combiner structure
//

#define CC_COMPONENTS   4  // ABCD, must be sizeof(uint32_t)
#define CC_CHANNELS     2  // RGB/A
#define CC_CYCLES       2  // Cycles

//
// Textures
//

#define MAX_TEXTURES    2

//
// CC sources and shader inputs
//

#define CCS_0           0
#define CCS_1           1
#define CCS_TEXEL0      2
#define CCS_TEXEL0A     3
#define CCS_TEXEL1      4
#define CCS_TEXEL1A     5
#define CCS_COMBINED    6
#define CCS_COMBINEDA   7
#define CCS_NOISE       8

#define CCS_PRIM        9
#define CCS_PRIMA       10
#define CCS_SHADE       11
#define CCS_SHADEA      12
#define CCS_ENV         13
#define CCS_ENVA        14
#define CCS_LOD         15

#define SHADER_INPUT_FIRST  CCS_PRIM
#define SHADER_INPUT_LAST   CCS_LOD
#define MAX_SHADER_INPUTS   (SHADER_INPUT_LAST - SHADER_INPUT_FIRST + 1)

//
// CC features
//

#define CCF_USE_ALPHA       0
#define CCF_USE_FOG         1
#define CCF_TEX_EDGE        2
#define CCF_USE_DITHER      3
#define CCF_USE_2CYCLE      4
#define CCF_LIGHT_MAP       5
#define CCF_WORLD_GEOMETRY  6
#define CCF_NOISE           7
#define CCF_USED_TEXTURE    8

#define USED_TEXTURE(i)  (1 << (i))
