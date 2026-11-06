//
// Keep everything in this file as C macros!
// All constants here are also included in the ubershader,
// and GLSL doesn't have enum types.
//

//
// Dot params
//

#define CC_EXT_DOT_PARAM_0      0
#define CC_EXT_DOT_PARAM_1      1
#define CC_EXT_DOT_PARAM_R      2
#define CC_EXT_DOT_PARAM_G      3
#define CC_EXT_DOT_PARAM_B      4
#define CC_EXT_DOT_PARAM_A      5

//
// Extended combiner structure
//

#define CC_EXT_MAX_OPS          128
#define CC_EXT_MAX_CONSTANTS    64
#define CC_EXT_MAX_REGISTERS    16
#define CC_EXT_STACK_SIZE       64
