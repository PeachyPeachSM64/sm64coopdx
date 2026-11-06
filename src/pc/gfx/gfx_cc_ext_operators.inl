//
// Extended CC operators
// ---------------------
// Unary and binary operators.
// Macro params are: (name, args, precedence, symbol)
//

CC_EXT_OPERATOR(END, 0, 0, ";")
CC_EXT_OPERATOR(DOT, 2, 0, ".")
CC_EXT_OPERATOR(NEG, 1, 1, "-")
CC_EXT_OPERATOR(NOT, 1, 1, "!")
CC_EXT_OPERATOR(POW, 2, 2, "^")
CC_EXT_OPERATOR(MUL, 2, 3, "*")
CC_EXT_OPERATOR(DIV, 2, 3, "/")
CC_EXT_OPERATOR(MOD, 2, 3, "%")
CC_EXT_OPERATOR(ADD, 2, 4, "+")
CC_EXT_OPERATOR(SUB, 2, 4, "-")
CC_EXT_OPERATOR(LT,  2, 5, "<")
CC_EXT_OPERATOR(LEQ, 2, 5, "<=")
CC_EXT_OPERATOR(GT,  2, 5, ">")
CC_EXT_OPERATOR(GEQ, 2, 5, ">=")
CC_EXT_OPERATOR(EQ,  2, 5, "==")
CC_EXT_OPERATOR(NEQ, 2, 5, "!=")
CC_EXT_OPERATOR(AND, 2, 6, "&&")
CC_EXT_OPERATOR(OR,  2, 6, "||")
CC_EXT_OPERATOR(SET, 2, 7, "=")
