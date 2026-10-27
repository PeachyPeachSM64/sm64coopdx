//
// Extended CC operators
// ---------------------
// Unary and binary operators.
// Macro params are: (name, args, precedence, symbol)
//

CC_EXT_OP(DOT, 3, 0, ".")
CC_EXT_OP(NEG, 1, 1, "-")
CC_EXT_OP(NOT, 1, 1, "!")
CC_EXT_OP(POW, 2, 2, "^")
CC_EXT_OP(MUL, 2, 3, "*")
CC_EXT_OP(DIV, 2, 3, "/")
CC_EXT_OP(MOD, 2, 3, "%")
CC_EXT_OP(ADD, 2, 4, "+")
CC_EXT_OP(SUB, 2, 4, "-")
CC_EXT_OP(LT,  2, 5, "<")
CC_EXT_OP(LEQ, 2, 5, "<=")
CC_EXT_OP(GT,  2, 5, ">")
CC_EXT_OP(GEQ, 2, 5, ">=")
CC_EXT_OP(EQ,  2, 5, "==")
CC_EXT_OP(NEQ, 2, 5, "!=")
CC_EXT_OP(AND, 2, 6, "&&")
CC_EXT_OP(OR,  2, 6, "||")
