// VALUES
// 0xyyyyyy
// x = 0 -> yyyyyy from 0 to 63 -> regular constant
// x = 1 -> 2 x 3 bits for dot param

// Note: the order does not follow the CCS constants!
// COMBINED sources are also missing, since there is no cycle 2 in extended combiners

// name, index

CC_EXT_VAL(0,       0)
CC_EXT_VAL(1,       1)
CC_EXT_VAL(TEXEL0,  2)
CC_EXT_VAL(TEXEL0A, 3)
CC_EXT_VAL(TEXEL1,  4)
CC_EXT_VAL(TEXEL1A, 5)
CC_EXT_VAL(PRIM,    6)
CC_EXT_VAL(PRIMA,   7)
CC_EXT_VAL(SHADE,   8)
CC_EXT_VAL(SHADEA,  9)
CC_EXT_VAL(ENV,     10)
CC_EXT_VAL(ENVA,    11)
CC_EXT_VAL(LOD,     12)
CC_EXT_VAL(NOISE,   13)

CC_EXT_VAL(CONST_FIRST, 14)
CC_EXT_VAL(CONST_LAST,  (CC_EXT_FLAG_VAL_DOT_PARAM - 1))


// OPS
// 10xxxyyy -> xxx = precedence from 0 to 7, yyy = op from 0 to 7
// 11xxxxxx -> xxxxxx = function id from 0 to 60 (0x3C) included (last 3 are reserved for parsing)

// name, symbol, index, precedence, num args

CC_EXT_OP(DOT, ".", 0, 0, 3)

CC_EXT_OP(NEG, "-", 0, 1, 1)
CC_EXT_OP(NOT, "!", 1, 1, 1)

CC_EXT_OP(POW, "^", 0, 2, 2)

CC_EXT_OP(MUL, "*", 0, 3, 2)
CC_EXT_OP(DIV, "/", 1, 3, 2)
CC_EXT_OP(MOD, "%", 2, 3, 2)

CC_EXT_OP(ADD, "+", 0, 4, 2)
CC_EXT_OP(SUB, "-", 1, 4, 2)

CC_EXT_OP(LT,  "<",  0, 5, 2)
CC_EXT_OP(LEQ, "<=", 1, 5, 2)
CC_EXT_OP(GT,  ">",  2, 5, 2)
CC_EXT_OP(GEQ, ">=", 3, 5, 2)
CC_EXT_OP(EQ,  "==", 4, 5, 2)
CC_EXT_OP(NEQ, "!=", 5, 5, 2)

CC_EXT_OP(AND, "&&", 0, 6, 2)
CC_EXT_OP(OR,  "||", 1, 6, 2)

// name, index, num args

CC_EXT_FUNC(VEC,      0x00, 4)
CC_EXT_FUNC(LERP,     0x01, 3)
CC_EXT_FUNC(MIN,      0x02, 2)
CC_EXT_FUNC(MAX,      0x03, 2)
CC_EXT_FUNC(CLAMP,    0x04, 3)
CC_EXT_FUNC(SATURATE, 0x05, 1)
CC_EXT_FUNC(COS,      0x06, 1)
CC_EXT_FUNC(SIN,      0x07, 1)
CC_EXT_FUNC(EXP,      0x08, 1)
CC_EXT_FUNC(SQRT,     0x09, 1)
CC_EXT_FUNC(RSQRT,    0x0A, 1)
CC_EXT_FUNC(ROUND,    0x0B, 1)
CC_EXT_FUNC(FLOOR,    0x0C, 1)
CC_EXT_FUNC(CEIL,     0x0D, 1)
