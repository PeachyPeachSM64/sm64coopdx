#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "game/memory.h"


#define CC_EXT_FLAG_OPERATOR            0x80
#define CC_EXT_FLAG_DOT_PARAM           0x40

#define CC_EXT_DOT_PARAM(op, x)         (((op) >> (3 * (x))) & 0x7)

#define CC_EXT_OPERATOR_ARGS_SHIFT      5
#define CC_EXT_OPERATOR_ARGS_WIDTH      2
#define CC_EXT_OPERATOR_ARGS_BITS       ((1 << CC_EXT_OPERATOR_ARGS_WIDTH) - 1)
#define CC_EXT_OPERATOR_ARGS(op)        ((((op) >> CC_EXT_OPERATOR_ARGS_SHIFT) & CC_EXT_OPERATOR_ARGS_BITS) + 1)


enum {
#define CC_EXT_VALUE(name) CC_EXT_ID_##name,
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
};

enum {
#define CC_EXT_OPERATOR_1(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR_2(name)
#define CC_EXT_OPERATOR_3(name)
#define CC_EXT_OPERATOR_4(name)
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION_2(name)
#define CC_EXT_FUNCTION_3(name)
#define CC_EXT_FUNCTION_4(name)
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};
enum {
#define CC_EXT_OPERATOR_1(name)
#define CC_EXT_OPERATOR_2(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR_3(name)
#define CC_EXT_OPERATOR_4(name)
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name)
#define CC_EXT_FUNCTION_2(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION_3(name)
#define CC_EXT_FUNCTION_4(name)
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};
enum {
#define CC_EXT_OPERATOR_1(name)
#define CC_EXT_OPERATOR_2(name)
#define CC_EXT_OPERATOR_3(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR_4(name)
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name)
#define CC_EXT_FUNCTION_2(name)
#define CC_EXT_FUNCTION_3(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION_4(name)
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};
enum {
#define CC_EXT_OPERATOR_1(name)
#define CC_EXT_OPERATOR_2(name)
#define CC_EXT_OPERATOR_3(name)
#define CC_EXT_OPERATOR_4(name) CC_EXT_ID_##name,
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_OPERATOR_##args(name)
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR_1
#undef CC_EXT_OPERATOR_2
#undef CC_EXT_OPERATOR_3
#undef CC_EXT_OPERATOR_4
#undef CC_EXT_OPERATOR
#define CC_EXT_FUNCTION_1(name)
#define CC_EXT_FUNCTION_2(name)
#define CC_EXT_FUNCTION_3(name)
#define CC_EXT_FUNCTION_4(name) CC_EXT_ID_##name,
#define CC_EXT_FUNCTION(name, args, ...) CC_EXT_FUNCTION_##args(name)
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION_1
#undef CC_EXT_FUNCTION_2
#undef CC_EXT_FUNCTION_3
#undef CC_EXT_FUNCTION_4
#undef CC_EXT_FUNCTION
};





enum {

// Values
#define CC_EXT_VALUE(name) CC_EXT_##name = CC_EXT_ID_##name,
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE
    CC_EXT_CONST_FIRST,
    CC_EXT_CONST_LAST = (CC_EXT_FLAG_DOT_PARAM - 1),

// Operators
#define CC_EXT_OPERATOR(name, args, ...) CC_EXT_##name = (CC_EXT_ID_##name | ((args - 1) << CC_EXT_OPERATOR_ARGS_SHIFT) | CC_EXT_FLAG_OPERATOR),
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, args) CC_EXT_##name = (CC_EXT_ID_##name | ((args - 1) << CC_EXT_OPERATOR_ARGS_SHIFT) | CC_EXT_FLAG_OPERATOR),
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

// Special ops for parsing
    CC_EXT_OPEN  = 0xFD,
    CC_EXT_CLOSE = 0xFE,
    CC_EXT_COMMA = 0xFF,

// Special op for validating
    CC_EXT_RESULT = 0xFF,
};

















#define CC_EXT_MAX_OPS           128
#define CC_EXT_MAX_CONSTANTS     (CC_EXT_CONST_LAST - CC_EXT_CONST_FIRST + 1)
#define CC_EXT_STACK_SIZE        32

struct ExtendedCombiner {
    uint8_t ops[CC_EXT_MAX_OPS];
    uint32_t num_ops;
    float constants[CC_EXT_MAX_CONSTANTS][4];
    uint32_t num_constants;
};






//
// Parsed token
//

typedef struct Token {
    char *str;
    size_t pos;
    size_t len;
    struct Token *prev;
} Token;

static void token_free(void *ptr) {
    Token *token = ptr;
    if (token) {
        free(token->str);
        free(token);
    }
}

static void token_push(struct GrowingArray *tokens, const char *src, size_t pos, size_t len) {
    if (len > 0) {
        Token *token = growing_array_alloc(tokens, sizeof(Token));
        token->str = memcpy(calloc(1, len + 1), src, len);
        token->pos = pos;
        token->len = len;
    }
}

#define token_push_curr() { \
    token_push(tokens, curr_token, curr_start, curr_len); \
    curr_len = 0; \
    is_number = false; \
    is_decimal = false; \
    is_dot_arg = false; \
}

#define token_is(_s_) \
    stricmp(token->str, _s_) == 0

#define token_to_operator(_s_, _op_, _is_unary_) { \
    if (token_is(_s_)) { \
        if (( (_is_unary_) && op_can_be_operator(prev)) || \
            (!(_is_unary_) && op_can_be_value(prev))) { \
            op_push(ops, (_op_), token); \
        } else { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), \
                "Operator `%s` at position %llu must follow %s.\n", \
                _s_, token->pos, (_is_unary_) ? "an operator" : "a value" \
            ); \
            gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        continue; \
    } \
}

#define token_to_function(_s_, _op_) { \
    if (token_is(_s_)) { \
        const Token *next_token = (_head_ + 1 < _tail_ ? *(_head_ + 1) : NULL); \
        if (!next_token || stricmp(next_token->str, "(") != 0) { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), \
                "Function `%s` at position %llu must be followed by a left parenthesis `(`.\n", \
                _s_, token->pos \
            ); \
            gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        if (op_can_be_operator(prev)) { \
            op_push(ops, (_op_), token); \
        } else { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), \
                "Function `%s` at position %llu must follow an operator.\n", \
                _s_, token->pos \
            ); \
            gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        continue; \
    } \
}

#define token_to_value(_s_, _op_) { \
    if (token_is(_s_)) { \
        if (op_can_be_operator(prev)) { \
            op_push(ops, (_op_), token); \
        } else { \
            snprintf(sErrorMsg, sizeof(sErrorMsg), \
                "Value `%s` at position %llu must follow an operator.\n", \
                _s_, token->pos \
            ); \
            gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0); \
            goto cleanup; \
        } \
        continue; \
    } \
}

static bool token_check_number(const Token *token) {
    size_t len = strlen(token->str);
    bool decimal = false;
    for (size_t i = 0; i < len; ++i) {
        char c = token->str[i];
        if (c == '.') {
            if (decimal) {
                return false;
            }
            decimal = true;
            continue;
        }
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

//
// Op with token
//

typedef struct Op {
    uint8_t op;
    const Token *token;
} Op;

static inline bool op_is_special(uint8_t op) {
    return op == CC_EXT_OPEN  ||
           op == CC_EXT_CLOSE ||
           op == CC_EXT_COMMA;
}

static inline bool op_is_operator(uint8_t op) {
    return !op_is_special(op) && (op & CC_EXT_FLAG_OPERATOR) != 0;
}

static inline bool op_is_function(uint8_t op) {
    static const bool IS_FUNCTION[0x100] = {
#define CC_EXT_FUNCTION(name, ...) [CC_EXT_##name] = true,
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION
    };
    return IS_FUNCTION[op];
}

static inline bool op_is_value(uint8_t op) {
    return (op & CC_EXT_FLAG_OPERATOR) == 0;
}

static inline bool op_is_constant(uint8_t op) {
    return op >= CC_EXT_CONST_FIRST && op <= CC_EXT_CONST_LAST;
}

static inline bool op_is_dot_param(uint8_t op) {
    return op_is_value(op) && (op & CC_EXT_FLAG_DOT_PARAM) != 0;
}

// A^B^C == A^(B^C)
static inline bool op_is_right_associative(uint8_t op) {
    return op == CC_EXT_POW;
}

static inline bool op_can_be_value(uint8_t op) {
    return op_is_value(op) || op == CC_EXT_CLOSE;
}

static inline bool op_can_be_operator(uint8_t op) {
    return op_is_operator(op) || op == CC_EXT_OPEN || op == CC_EXT_COMMA;
}

// Follow simple precedence rules as in https://en.cppreference.com/c/language/operator_precedence.
// Functions and dot operator have precedence 0.
static inline uint8_t op_get_precedence(uint8_t op) {
    static const uint8_t OP_PRECEDENCE[0x100] = {
#define CC_EXT_OPERATOR(name, args, precedence, ...) [CC_EXT_##name] = precedence,
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR
    };
    return OP_PRECEDENCE[op];
}

static inline void op_push(struct GrowingArray *ops, uint8_t op, const Token *token) {
    Op *new_op = growing_array_alloc(ops, sizeof(Op));
    new_op->op = op;
    new_op->token = token;
}

static inline Op *op_get(struct GrowingArray *ops, uint32_t index) {
    return (Op *) ops->buffer[index];
}

static Op *op_get_from_token(struct GrowingArray *ops, const Token *token) {
    if (!token) { return NULL; }
    growing_array_for_each_(ops, Op, op) {
        if (op->token == token) {
            return op;
        }
    }
    return NULL;
}

static int op_get_index(struct GrowingArray *ops, const Op *op) {
    if (!op) { return -1; }
    for (uint32_t i = 0; i < ops->count; ++i) {
        if (op_get(ops, i) == op) {
            return i;
        }
    }
    return -1;
}

static const char *op_to_string(uint8_t op) {
static char OP_TO_STR[0x100][32] = {
#define CC_EXT_VALUE(name)         [CC_EXT_##name] = "v_" #name,
#define CC_EXT_OPERATOR(name, ...) [CC_EXT_##name] = "o_" #name,
#define CC_EXT_FUNCTION(name, ...) [CC_EXT_##name] = "f_" #name,
#include "gfx_cc_ext_values.inl"
#include "gfx_cc_ext_operators.inl"
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_VALUE
#undef CC_EXT_OPERATOR
#undef CC_EXT_FUNCTION
    [CC_EXT_OPEN]  = "(",
    [CC_EXT_CLOSE] = ")",
    [CC_EXT_COMMA] = ",",
};
    char *str = OP_TO_STR[op];
    if (*str) {
        return str;
    }
    if (op_is_constant(op)) {
        snprintf(str, sizeof(*OP_TO_STR), "v_CONST_%02d", op - CC_EXT_CONST_FIRST);
        return str;
    }
    if (op_is_dot_param(op)) {
        snprintf(str, sizeof(*OP_TO_STR), "%c%c", "rgba0111"[CC_EXT_DOT_PARAM(op, 0)], "rgba0111"[CC_EXT_DOT_PARAM(op, 1)]);
        return str;
    }
    return str;
}

//
// Constant as float4
//

typedef struct Constant {
    float x, y, z, w;
} Constant;

static inline void constant_push(struct GrowingArray *constants, float constant) {
    Constant *new_constant = growing_array_alloc(constants, sizeof(Constant));
    new_constant->x = constant;
    new_constant->y = constant;
    new_constant->z = constant;
    new_constant->w = constant;
}

static inline void constant_push_4(struct GrowingArray *constants, float x, float y, float z, float w) {
    Constant *new_constant = growing_array_alloc(constants, sizeof(Constant));
    new_constant->x = x;
    new_constant->y = y;
    new_constant->z = z;
    new_constant->w = w;
}

static inline Constant *constant_get(struct GrowingArray *constants, uint32_t index) {
    return (Constant *) constants->buffer[index];
}

static int constant_get_index(struct GrowingArray *constants, float value) {
    for (uint32_t i = 0; i < constants->count; ++i) {
        if (constant_get(constants, i)->x == value) {
            return i;
        }
    }
    return -1;
}

static int constant_get_index_4(struct GrowingArray *constants, float x, float y, float z, float w) {
    for (uint32_t i = 0; i < constants->count; ++i) {
        const Constant *constant = constant_get(constants, i);
        if (constant->x == x && constant->y == y &&
            constant->z == z && constant->w == w) {
            return i;
        }
    }
    return -1;
}

//
// Operation result
//

typedef struct Result {
    uint8_t op;
    struct Result *args[4];
} Result;





//
// Debug and logging
//



static void gfx_cc_ext_print_error_location(char *error_msg, size_t msg_len, const char *str, size_t pos, size_t len, size_t offset) {
    size_t curr_len = strlen(error_msg);
    error_msg += curr_len;
    msg_len -= curr_len;

    size_t written_chars = snprintf(error_msg, msg_len, "%s\n%*s", str, (int) pos, "");
    error_msg += written_chars;
    msg_len -= written_chars;

    size_t index = 0;
    while (msg_len > 1 && index < len) {
        *(error_msg++) = (index == offset ? '^' : '~');
        msg_len--;
        index++;
    }

    *error_msg = 0;
}

static void gfx_cc_ext_print_ops(struct GrowingArray *ops, struct GrowingArray *constants) {
    growing_array_for_each_(ops, const Op, op) {
        printf("%s ", op_to_string(op->op));
    }
    printf("\n");
    for (uint32_t i = 0; i < constants->count; ++i) {
        const Constant *constant = constant_get(constants, i);
        printf("v_CONST_%02X: %f %f %f %f\n", i, constant->x, constant->y, constant->z, constant->w);
    }
}

static void gfx_cc_ext_check_duplicates() {
    uint8_t counts[0x100] = {0};

// Values
#define CC_EXT_VALUE(name) counts[CC_EXT_##name]++;
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE

// Constants
    for (int i = CC_EXT_CONST_FIRST; i <= CC_EXT_CONST_LAST; ++i) {
        counts[i]++;
    }

// Dot params
    for (int i = CC_EXT_FLAG_DOT_PARAM; i < CC_EXT_FLAG_OPERATOR; ++i) {
        counts[i]++;
    }

// Operators
#define CC_EXT_OPERATOR(name, args, ...) counts[CC_EXT_##name]++;
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, args) counts[CC_EXT_##name]++;
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

// Special ops for parsing
    counts[CC_EXT_OPEN]++;
    counts[CC_EXT_CLOSE]++;
    counts[CC_EXT_COMMA]++;

    printf("\n");
    printf("\e[0;32mGreen\e[0m number indicates that the slot is taken.\n");
    printf("\e[0;36mC\e[0m are available constants.\n");
    printf("\e[0;33mD\e[0m are dot operator parameter combinations.\n");
    printf("\e[0;35mS\e[0m are special ops, reserved for parsing.\n");
    printf("\e[0;31mRed\e[0m number indicates a duplicate.\n");
    printf("\e[0;31mMAKE SURE NO RED NUMBER APPEARS IS THE FOLLOWING TABLE.\e[0m\n");
    printf("\n");
    printf("     0 1 2 3 4 5 6 7 8 9 A B C D E F\n");
    printf("   +--------------------------------");
    for (int i = 0; i < 0x100; ++i) {
        if (i % 16 == 0) {
            printf("\n%Xx |", i / 16);
        };
        switch (counts[i]) {
            case 0: printf(" ."); break;
            case 1: if (op_is_dot_param(i)) {
                        printf(" \e[0;33mD\e[0m");
                    } else if (op_is_value(i) && i >= CC_EXT_CONST_FIRST) {
                        printf(" \e[0;36mC\e[0m");
                    } else if (op_is_special(i)) {
                        printf(" \e[0;35mS\e[0m");
                    } else {
                        printf(" \e[0;32m1\e[0m");
                    }
                    break;
            default: printf(" \e[0;31m%d\e[0m", counts[i]); break;
        }
    }
    printf("\n");
    printf("\n");
}





//
// Infix to prefix
//

static inline void stack_push(struct GrowingArray *stack, Op *op) {
    op_push(stack, op->op, op->token);
}

static inline Op *stack_top(struct GrowingArray *stack) {
    return op_get(stack, stack->count - 1);
}

static inline Op *stack_pop(struct GrowingArray *stack) {
    Op *op = stack_top(stack);
    stack->count--;
    return op;
}

static struct GrowingArray *gfx_cc_ext_generate_prefix_ops(struct GrowingArray *ops, char *error_msg, size_t msg_len, const char *str) {
    struct GrowingArray *res   = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *stack = growing_array_init(NULL, 16, malloc, free);
    bool ok = false;

    // from right to left
    for (int i = ops->count - 1; i >= 0; --i) {
        Op *op = op_get(ops, i);

        if (op_is_value(op->op)) {
            op_push(res, op->op, op->token);
            continue;
        }

        if (op->op == CC_EXT_CLOSE) {
            stack_push(stack, op);
            continue;
        }

        if (op->op == CC_EXT_OPEN) {
            while (stack->count > 0 && stack_top(stack)->op != CC_EXT_CLOSE) {
                Op *top = stack_pop(stack);
                op_push(res, top->op, top->token);
            }

            // push function
            if (i > 0) {
                Op *prev = op_get(ops, i - 1);
                if (op_is_function(prev->op)) {
                    op_push(res, prev->op, prev->token);
                    i--;
                }
            }

            // should pop ')'
            if (stack->count == 0 || stack_top(stack)->op != CC_EXT_CLOSE) {
                if (op->token) {
                    snprintf(error_msg, msg_len,
                        "Missing right parenthesis `)` matching left parenthesis `(` at position %llu.\n",
                        op->token->pos
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                } else {
                    snprintf(error_msg, msg_len,
                        "Missing right parenthesis `)`."
                    );
                }
                goto cleanup;
            } else {
                stack_pop(stack);
            }
            continue;
        }

        if (op_is_operator(op->op)) {
            Op *top;
            while (
                stack->count > 0 &&
                (top = stack_top(stack)) &&
                op_is_operator(top->op) && (
                op_get_precedence(top->op) < op_get_precedence(op->op) || (
                op_get_precedence(top->op) == op_get_precedence(op->op) && op_is_right_associative(op->op)
            ))) {
                top = stack_pop(stack);
                op_push(res, top->op, top->token);
            }
            stack_push(stack, op);
            continue;
        }

        if (op->op != CC_EXT_COMMA) {
            if (op->token) {
                snprintf(error_msg, msg_len,
                    "Invalid op 0x%02X from token at position %llu: `%s`\n",
                    op->op, op->token->pos, op->token->str
                );
                gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
            } else {
                snprintf(error_msg, msg_len,
                    "Invalid op 0x%02X.",
                    op->op
                );
            }
            goto cleanup;
        }
    }

    // pop remaining ops
    while (stack->count > 0) {
        Op *op = stack_pop(stack);
        op_push(res, op->op, op->token);
    }
    ok = true;

cleanup:
    growing_array_free(&stack);
    if (!ok) {
        growing_array_free(&res);
        return NULL;
    }
    return res;
}

//
// Validation
//

static bool gfx_cc_ext_validate(struct GrowingArray *ops, struct GrowingArray *constants, char *error_msg, size_t msg_len, const char *str) {
    static const Op sResultOp = { CC_EXT_RESULT, NULL };
    const Op *stack[CC_EXT_STACK_SIZE] = {0};
    size_t stack_size = 0;

    for (uint32_t i = 0; i < ops->count; ++i) {
        const Op *op = op_get(ops, i);

        // values
        if (op_is_value(op->op)) {

            // check if constant is defined
            if (op_is_constant(op->op)) {
                uint8_t constant_index = op->op - CC_EXT_CONST_FIRST;
                if (constant_index >= constants->count) {
                    if (op->token) {
                        snprintf(error_msg, msg_len,
                            "Validation failed: Constant %u at position %llu is not defined, there is only %u constants loaded.\n",
                            constant_index, op->token->pos, constants->count
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                    } else {
                        snprintf(error_msg, msg_len,
                            "Validation failed: Constant %u is not defined, there is only %u constants loaded.\n",
                            constant_index, constants->count
                        );
                    }
                    return false;
                }
            }

            // check stack size before push
            if (stack_size == CC_EXT_STACK_SIZE) {
                if (op->token) {
                    snprintf(error_msg, msg_len,
                        "Validation failed: Stack is full, cannot push value at position %llu: `%s`\n",
                        op->token->pos, op->token->str
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                } else {
                    snprintf(error_msg, msg_len,
                        "Validation failed: Stack is full, cannot push more values."
                    );
                }
                return false;
            }

            stack[stack_size++] = op;
            continue;
        }

        // operators
        if (op_is_operator(op->op)) {
            uint8_t args = CC_EXT_OPERATOR_ARGS(op->op);

            // check if there is enough values in stack
            if (stack_size < args) {
                if (op->token) {
                    snprintf(error_msg, msg_len,
                        "Validation failed: Not enough values in stack for %s at position %llu: `%s` (%llu values, should be at least %u)\n",
                        op_is_function(op->op) ? "function" : "operator", op->token->pos, op->token->str, stack_size, args
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                } else {
                    snprintf(error_msg, msg_len,
                        "Validation failed: Not enough values in stack for %s: `%s` (%llu values, should be at least %u)\n",
                        op_is_function(op->op) ? "function" : "operator", op_to_string(op->op), stack_size, args
                    );
                }
                return false;
            }

            // check operator args
            for (uint8_t k = 0; k < args; ++k) {
                const Op *arg = stack[stack_size - (1 + k)];

                // arg must be a value or a result
                if (!op_is_value(arg->op) && arg != &sResultOp) {
                    if (op->token) {
                        snprintf(error_msg, msg_len,
                            "Validation failed: Argument %u of %s `%s` at position %llu is not a value: `%s`\n",
                            k, op_is_function(op->op) ? "function" : "operator", op->token->str, op->token->pos, arg->token ? arg->token->str : op_to_string(arg->op)
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, arg->token ? arg->token->pos : op->token->pos, arg->token ? arg->token->len : op->token->len, 0);
                    } else {
                        snprintf(error_msg, msg_len,
                            "Validation failed: Argument %u of %s `%s` is not a value: `%s`\n",
                            k, op_is_function(op->op) ? "function" : "operator", op_to_string(op->op), arg->token ? arg->token->str : op_to_string(arg->op)
                        );
                    }
                    return false;
                }

                // args 2 and 3 of dot operator must be dot params
                if (op->op == CC_EXT_DOT && (k == 1 || k == 2)) {
                    if (!op_is_dot_param(arg->op)) {
                        if (op->token) {
                            snprintf(error_msg, msg_len,
                                "Validation failed: Argument %u of dot operator `.` at position %llu is not a dot param: `%s`\n",
                                k, op->token->pos, arg->token ? arg->token->str : op_to_string(arg->op)
                            );
                            gfx_cc_ext_print_error_location(error_msg, msg_len, str, arg->token ? arg->token->pos : op->token->pos, arg->token ? arg->token->len : op->token->len, 0);
                        } else {
                            snprintf(error_msg, msg_len,
                                "Validation failed: Argument %u of dot operator `.` is not a dot param: `%s`\n",
                                k, arg->token ? arg->token->str : op_to_string(arg->op)
                            );
                        }
                        return false;
                    }
                }

                // otherwise, arg must not be a dot param
                else if (op_is_dot_param(arg->op)) {
                    if (op->token) {
                        snprintf(error_msg, msg_len,
                            "Validation failed: Argument %u of %s `%s` at position %llu must not be a dot param: `%s`\n",
                            k, op_is_function(op->op) ? "function" : "operator", op->token->str, op->token->pos, arg->token ? arg->token->str : op_to_string(arg->op)
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, arg->token ? arg->token->pos : op->token->pos, arg->token ? arg->token->len : op->token->len, 0);
                    } else {
                        snprintf(error_msg, msg_len,
                            "Validation failed: Argument %u of %s `%s` must not be a dot param: `%s`\n",
                            k, op_is_function(op->op) ? "function" : "operator", op_to_string(op->op), arg->token ? arg->token->str : op_to_string(arg->op)
                        );
                    }
                    return false;
                }
            }

            stack_size -= args; // pop args
            stack[stack_size++] = &sResultOp; // push result
            continue;
        }

        // invalid op
        if (op->token) {
            snprintf(error_msg, msg_len,
                "Validation failed: Invalid op 0x%02X from token at position %llu: `%s`\n",
                op->op, op->token->pos, op->token->str
            );
            gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
        } else {
            snprintf(error_msg, msg_len,
                "Validation failed: Invalid op 0x%02X.",
                op->op
            );
        }
        return false;
    }

    // empty combiner
    if (stack_size == 0) {
        snprintf(error_msg, msg_len,
            "Validation failed: Combiner is empty, no result in stack."
        );
        return false;
    }

    // too much values in stack
    if (stack_size > 1) {
        snprintf(error_msg, msg_len,
            "Validation failed: Combiner is ill-formed, too much values (%llu) remain in stack after processing.",
            stack_size
        );
        return false;
    }

    return true;
}

//
// Optimizations
//










//
// Parsing
//

static size_t check_symbols(const char *str, size_t len, size_t offset) {
    static struct { const char *str; size_t len; } SYMBOLS[] = {
#define CC_EXT_OPERATOR(name, args, precedence, symbol) { symbol, sizeof(symbol) - 1 },
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR
        { "(", 1 },
        { ")", 1 },
        { ",", 1 },
    };

    size_t max_size = 0;
    for (int i = 0; i < ARRAY_COUNT(SYMBOLS); ++i) {
        size_t symb_len = SYMBOLS[i].len;
        if (max_size < symb_len && offset + symb_len <= len && strnicmp(str + offset, SYMBOLS[i].str, SYMBOLS[i].len) == 0) {
            max_size = symb_len;
        }
    }
    return max_size;
}

static const char *gfx_cc_ext_parse_combiner(const char *str, struct ExtendedCombiner *comb) {
    static char sErrorMsg[1024];

    struct GrowingArray *tokens    = growing_array_init(NULL, 16, malloc, token_free);
    struct GrowingArray *ops       = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *constants = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *prefix    = NULL;
    bool ok = false;

    // tokenize input
    bool is_number = false;
    bool is_decimal = false;
    bool is_dot_arg = false;
    char curr_token[32];
    size_t curr_start = 0;
    size_t curr_len = 0;
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];

        // ignore whitespaces and others
        if (iscntrl(c) || isspace(c)) {
            token_push_curr();
            continue;
        }

        // symbols
        size_t symb_max_size = check_symbols(str, len, i);
        if (symb_max_size > 0 && (symb_max_size > 1 || c != '.' || !is_number)) {
            token_push_curr();
            token_push(tokens, str + i, i, symb_max_size);
            i += (symb_max_size - 1);
            is_dot_arg = (symb_max_size == 1 && c == '.');
            continue;
        }

        // check token length
        if (curr_len == sizeof(curr_token)) {
            snprintf(sErrorMsg, sizeof(sErrorMsg),
                "Token at position %llu is too long.\n",
                i - curr_len
            );
            gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, i - curr_len, curr_len, 0);
            goto cleanup;
        }

        // number
        if (!is_dot_arg && isdigit(c) && (is_number || curr_len == 0)) {
            if (curr_len == 0) { curr_start = i; }
            curr_token[curr_len++] = c;
            is_number = true;
            continue;
        }
        if (is_number && c == '.') {
            if (is_decimal) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Number at position %llu cannot have two or more decimal points.\n",
                    i - curr_len
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, i - curr_len, curr_len + 1, curr_len);
                goto cleanup;
            }
            if (curr_len == 0) { curr_start = i; }
            curr_token[curr_len++] = c;
            is_decimal = true;
            continue;
        }

        // named value
        if (!is_number && (isalpha(c) || isdigit(c) || c == '_')) {
            if (curr_len == 0) { curr_start = i; }
            curr_token[curr_len++] = c;
            continue;
        }

        // invalid char
        snprintf(sErrorMsg, sizeof(sErrorMsg),
            "Invalid character at position %llu:  %c  (%d, 0x%02X)\n",
            i, c, c, c
        );
        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, i, 1, 0);
        goto cleanup;
    }

    // push remaining token
    token_push_curr();

    // printf("===============================\n");
    // printf("tokens: %u\n", tokens->count);
    // growing_array_for_each_(tokens, const Token, token) {
    //     printf("%s\n", token->str);
    // }
    // printf("===============================\n");


    // turn tokens into ops
    Token *scope = NULL;
    growing_array_for_each_(tokens, Token, token) {
        uint8_t prev = (ops->count > 0 ? op_get(ops, ops->count - 1)->op : CC_EXT_COMMA);

        // manually checked symbols
        // includes unary plus and minus, parentheses and comma ops

        if (token_is("+")) {
            if (op_can_be_value(prev)) {
                op_push(ops, CC_EXT_ADD, token);
            } else {
                // ignore unary plus
            }
            continue;
        }

        if (token_is("-")) {
            if (op_can_be_value(prev)) {
                op_push(ops, CC_EXT_SUB, token);
            } else {
                op_push(ops, CC_EXT_NEG, token);
            }
            continue;
        }

        if (token_is("(")) {
            token->prev = scope;
            scope = token;
            if (op_can_be_operator(prev)) {
                op_push(ops, CC_EXT_OPEN, token);
            } else {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Left parenthesis `(` at position %llu must follow an operator.\n",
                    token->pos
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            continue;
        }

        if (token_is(")")) {
            if (!op_can_be_value(prev)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Right parenthesis `)` at position %llu must follow a value.\n",
                    token->pos
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            if (!scope) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Too much right parentheses `)` at position %llu.\n",
                    token->pos
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            op_push(ops, CC_EXT_CLOSE, token);
            scope = scope->prev;
            continue;
        }

        if (token_is(",")) {
            if (!op_can_be_value(prev)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Function parameter separator `,` at position %llu must follow a value.\n",
                    token->pos
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            // check if inside function
            // get current scope, check previous op
            const Op *op = op_get_from_token(ops, scope);
            int index = op_get_index(ops, op);
            if (index <= 0 || !op_is_function(op_get(ops, index - 1)->op)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Function parameter separator `,` at position %llu cannot be used outside a function.\n",
                    token->pos
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            op_push(ops, CC_EXT_COMMA, token);
            continue;
        }

        // operators
#define CC_EXT_OPERATOR(name, args, precedence, symbol) token_to_operator(symbol, CC_EXT_##name, precedence == 1);
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

        // functions
#define CC_EXT_FUNCTION(name, args) token_to_function(#name, CC_EXT_##name);
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

        // values
#define CC_EXT_VALUE(name) token_to_value(#name, CC_EXT_##name);
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE

        // other values
        token_to_value("0.0", CC_EXT_0);
        token_to_value("1.0", CC_EXT_1);
        token_to_value("TEXEL0_ALPHA", CC_EXT_TEXEL0A);
        token_to_value("TEXEL1_ALPHA", CC_EXT_TEXEL1A);
        token_to_value("PRIMITIVE", CC_EXT_PRIM);
        token_to_value("PRIMITIVE_ALPHA", CC_EXT_PRIMA);
        token_to_value("SHADE_ALPHA", CC_EXT_SHADEA);
        token_to_value("ENVIRONMENT", CC_EXT_ENV);
        token_to_value("ENV_ALPHA", CC_EXT_ENVA);
        token_to_value("LOD_FRACTION", CC_EXT_LOD);

        // dot
        if (ops->count > 0 && prev == CC_EXT_DOT) {
            size_t len = strlen(token->str);
            if (len != 1 && len != 3 && len != 4) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Operator `.` parameter `%s` at position %llu has invalid length: %llu (must be 1, 3 or 4).\n",
                    token->str, token->pos, len
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            uint8_t dot_params[2] = { CC_EXT_FLAG_DOT_PARAM, CC_EXT_FLAG_DOT_PARAM };
            for (size_t i = 0; i < 4; ++i) {
                char c = (
                    i < len ? token->str[i] : (
                    len == 1 ? token->str[0] : (
                    len == 3 ? '1' : '0'
                )));
                uint8_t k = 0;
                switch (c) {
                    case 'r': case 'R': k = 0; break;
                    case 'g': case 'G': k = 1; break;
                    case 'b': case 'B': k = 2; break;
                    case 'a': case 'A': k = 3; break;
                    case '0':           k = 4; break;
                    case '1':           k = 5; break;
                    default:
                        snprintf(sErrorMsg, sizeof(sErrorMsg),
                            "Operator `.` parameter `%s` at position %llu has invalid character:  %c  (%d, 0x%02X) (must be one of \"rgba01\").\n",
                            token->str, token->pos, c, c, c
                        );
                        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, i);
                        goto cleanup;
                }
                dot_params[i / 2] |= (k << (3 * (i % 2)));
            }

            op_push(ops, dot_params[0], token);
            op_push(ops, dot_params[1], token);
            continue;
        }

        // numbers and math constants
        float constant;
        if ((token_check_number(token) && sscanf(token->str, "%f", &constant) == 1) ||
            (token_is("PI") && (constant = M_PI)) ||
            (token_is("E") && (constant = M_E))) {
            if (!op_can_be_operator(prev)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Constant `%f` at position %llu must follow an operator.\n",
                    constant, token->pos
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }

            // handle 0 and 1
            if (constant == 0) {
                op_push(ops, CC_EXT_0, token);
                continue;
            }
            if (constant == 1) {
                op_push(ops, CC_EXT_1, token);
                continue;
            }

            // don't duplicate constants
            int index = constant_get_index(constants, constant);
            if (index >= 0) {
                op_push(ops, CC_EXT_CONST_FIRST + index, token);
            } else if (constants->count < CC_EXT_MAX_CONSTANTS) {
                op_push(ops, CC_EXT_CONST_FIRST + constants->count, token);
                constant_push(constants, constant);
            } else {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Max number of constants reached: %u\n",
                    CC_EXT_MAX_CONSTANTS
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
            continue;
        }

        // invalid
        snprintf(sErrorMsg, sizeof(sErrorMsg),
            "Invalid token at position %llu: `%s`\n",
            token->pos, token->str
        );
        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
        goto cleanup;
    }

    // check for missing right parentheses
    if (scope) {
        snprintf(sErrorMsg, sizeof(sErrorMsg),
            "Unclosed left parenthesis `(` at position %llu.\n",
            scope->pos
        );
        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, scope->pos, scope->len, 0);
        goto cleanup;
    }

    gfx_cc_ext_print_ops(ops, constants);
    printf("\n");

    prefix = gfx_cc_ext_generate_prefix_ops(ops, sErrorMsg, sizeof(sErrorMsg), str);
    if (!prefix) {
        goto cleanup;
    }

    gfx_cc_ext_print_ops(prefix, constants);
    printf("\n");

    // validate
    if (!gfx_cc_ext_validate(prefix, constants, sErrorMsg, sizeof(sErrorMsg), str)) {
        goto cleanup;
    }

    // optimize


    // // check sizes
    // if (prefix->count > CC_EXT_MAX_OPS) {
    //     printf("Max number of ops exceeded: %d > %d\n", prefix->count, CC_EXT_MAX_OPS);
    //     goto cleanup;
    // }
    // if (constants->count > CC_EXT_MAX_CONSTANTS) {
    //     printf("Max number of constants exceeded: %d > %d\n", constants->count, CC_EXT_MAX_CONSTANTS);
    //     goto cleanup;
    // }

    // // fill struct
    // memset(comb, 0, sizeof(*comb));
    // comb->num_ops = prefix->count;
    // for (uint32_t i = 0; i < prefix->count; ++i) {
    //     comb->ops[i] = op_get(prefix, i);
    // }
    // comb->num_constants = constants->count;
    // for (uint32_t i = 0; i < constants->count; ++i) {
    //     comb->constants[i][0] = constant_get(constants, i);
    // }
    ok = true;

cleanup:
    growing_array_free(&tokens);
    growing_array_free(&ops);
    growing_array_free(&constants);
    growing_array_free(&prefix);
    return ok ? NULL : sErrorMsg;
}


static const char *error_checks[] = {
"TEXEL0 ! TEXEL1",
"TEXEL0 + * TEXEL1",
"lerp -TEXEL0, TEXEL1, ENV",
"(TEXEL0) TEXEL1",
"TEXEL00000000000000000000000000000000000",
"12.5.2",
"[]",
"TEXEL0 (TEXEL1)",
"(TEXEL0 + )",
"(TEXEL0))",
"lerp(, TEXEL0)",
"(TEXEL0, TEXEL1)",
"lerp(TEXEL0, (TEXEL1, ENV))",
"TEXEL0.rg",
"TEXEL0.abcd",
"TEXEL0 1.25",
"TEXEL2",
"lerp(TEXEL0",


};




__attribute__((constructor)) static void test_custom_combine() {

    const char *comb_str = 
    "ENV * (-TEXEL0 + TEXEL1.a).gb1 + (-0.5 >= 0.2) * 99 + vec(9, 10, 11, 12) - min(0.5, max(lerp(PRIM + ENVA * 0.75, SHADE, PRIMA).00ra.gbra, 0)) + 2 * 1 - 1.5 ^ (3 + 1)";


    // for (int i = 0; i < ARRAY_COUNT(error_checks); ++i) {

    // const char *comb_str = error_checks[i];

    struct ExtendedCombiner comb;
    const char *err = gfx_cc_ext_parse_combiner(comb_str, &comb);

    // printf("\n");
    if (err) {
        printf("%s\n\n", err);
    }

    // }

    // gfx_cc_ext_check_duplicates();

    exit(0);
}

