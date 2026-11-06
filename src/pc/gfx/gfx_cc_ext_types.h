#ifndef GFX_CC_EXT_TYPES_H
#define GFX_CC_EXT_TYPES_H

#include <ctype.h>

#include "engine/math_util.h"
#include "game/memory.h"

// Global flags for debugging
#ifdef DEVELOPMENT
#define GFX_CC_EXT_PRINT_OPS_AFTER_PARSING          1
#define GFX_CC_EXT_DISABLE_OPTIMIZATION             0
#define GFX_CC_EXT_PRINT_OPTIMIZATION               1
#define GFX_CC_EXT_PRINT_OPS_AFTER_OPTIMIZATION     1
#endif

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
            op_push(ops, (_op_), false, token); \
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
            op_push(ops, (_op_), false, token); \
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
            op_push(ops, (_op_), false, token); \
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
// Constant as float4
//

typedef float float4[4];

typedef struct Constant {
    union {
        struct { float x, y, z, w; };
        float4 xyzw;
    };
} Constant;

#define float4_eq(a, b) \
    (memcmp(a, b, sizeof(float4)) == 0)

static inline void constant_push(struct GrowingArray *constants, float constant) {
    Constant *new_constant = growing_array_alloc(constants, sizeof(Constant));
    new_constant->x = constant;
    new_constant->y = constant;
    new_constant->z = constant;
    new_constant->w = constant;
}

static inline void constant_push_4(struct GrowingArray *constants, const float4 xyzw) {
    Constant *new_constant = growing_array_alloc(constants, sizeof(Constant));
    new_constant->x = xyzw[0];
    new_constant->y = xyzw[1];
    new_constant->z = xyzw[2];
    new_constant->w = xyzw[3];
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

static int constant_get_index_4(struct GrowingArray *constants, const float4 xyzw) {
    for (uint32_t i = 0; i < constants->count; ++i) {
        const Constant *constant = constant_get(constants, i);
        if (float4_eq(constant->xyzw, xyzw)) {
            return i;
        }
    }
    return -1;
}

static int constant_get_builtin_value(const float4 xyzw) {
    static const struct { float4 xyzw; uint8_t op; } BUILTIN_CONSTANTS[] = {
        { { 0, 0, 0, 0 }, CC_EXT_0 },
        { { 1, 1, 1, 1 }, CC_EXT_1 },
    };
    for (int i = 0; i < ARRAY_COUNT(BUILTIN_CONSTANTS); ++i) {
        if (float4_eq(xyzw, BUILTIN_CONSTANTS[i].xyzw)) {
            return BUILTIN_CONSTANTS[i].op;
        }
    }
    return -1;
}

//
// Op with token
//

typedef struct Op {
    uint8_t op;
    bool is_dot_param;
    const Token *token;
} Op;

static struct {
    bool is_operator;
    bool is_function;
    uint8_t precedence;
    uint8_t num_args;
} OP_PROPERTIES[0x100] = {

// Values
#define CC_EXT_VALUE(name) [CC_EXT_##name] = { false, false, 0, 0 },
#include "gfx_cc_ext_values.inl"
#undef CC_EXT_VALUE

// Operators
#define CC_EXT_OPERATOR(name, args, precedence, ...) [CC_EXT_##name] = { true, false, precedence, args },
#include "gfx_cc_ext_operators.inl"
#undef CC_EXT_OPERATOR

// Functions
#define CC_EXT_FUNCTION(name, args) [CC_EXT_##name] = { true, true, 0, args },
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_FUNCTION

};

static inline bool op_is_special(uint8_t op) {
    return op >= CC_EXT_SPECIAL;
}

static inline bool op_is_operator(uint8_t op) {
    return !op_is_special(op) && OP_PROPERTIES[op].is_operator;
}

static inline bool op_is_function(uint8_t op) {
    return !op_is_special(op) && OP_PROPERTIES[op].is_function;
}

static inline bool op_is_value(uint8_t op) {
    return !op_is_special(op) && !OP_PROPERTIES[op].is_operator;
}

static inline bool op_is_constant(uint8_t op) {
    return op >= CC_EXT_CONST_FIRST && op <= CC_EXT_CONST_LAST;
}

static inline bool op_is_register(uint8_t op) {
    return op >= CC_EXT_REGISTER_FIRST && op <= CC_EXT_REGISTER_LAST;
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
    return OP_PROPERTIES[op].precedence;
}

static inline uint8_t op_get_num_args(uint8_t op) {
    return OP_PROPERTIES[op].num_args;
}

static inline void op_push(struct GrowingArray *ops, uint8_t op, bool is_dot_param, const Token *token) {
    Op *new_op = growing_array_alloc(ops, sizeof(Op));
    new_op->op = op;
    new_op->is_dot_param = is_dot_param;
    new_op->token = token;
}

static bool op_push_constant(struct GrowingArray *ops, struct GrowingArray *constants, bool is_dot_param, float4 constant, const Token *token) {

    // handle 0 and 1
    int value_op = constant_get_builtin_value(constant);
    if (value_op >= 0) {
        op_push(ops, value_op, is_dot_param, token);
        return true;
    }

    // don't duplicate constants
    int index = constant_get_index_4(constants, constant);
    if (index >= 0) {
        op_push(ops, CC_EXT_CONST_FIRST + index, is_dot_param, token);
    } else if (constants->count < CC_EXT_MAX_CONSTANTS) {
        op_push(ops, CC_EXT_CONST_FIRST + constants->count, is_dot_param, token);
        constant_push_4(constants, constant);
    } else {
        return false;
    }
    return true;
}

static inline Op *op_get(struct GrowingArray *ops, uint32_t index) {
    return (Op *) (index < ops->count ? ops->buffer[index] : NULL);
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

static bool op_to_float4(uint8_t op, struct GrowingArray *constants, float4 output) {
    if (op_is_constant(op)) {
        const Constant *constant = constant_get(constants, op - CC_EXT_CONST_FIRST);
        memcpy(output, constant->xyzw, sizeof(constant->xyzw));
        return true;
    }
    if (op == CC_EXT_0) {
        output[0] = output[1] = output[2] = output[3] = 0;
        return true;
    }
    if (op == CC_EXT_1) {
        output[0] = output[1] = output[2] = output[3] = 1;
        return true;
    }
    return false;
}

static const char *op_to_string(const Op *op, struct GrowingArray *constants) {
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

    if (op->is_dot_param) {
        static char sDotParamStr[8];
        float4 xyzw;
        if (op_to_float4(op->op, constants, xyzw)) {
            for (int i = 0; i < 4; ++i) {
                int k = (int) xyzw[i];
                sDotParamStr[i] = (k >= 0 && k < 6 ? "01rgba"[k] : '?');
            }
            sDotParamStr[4] = 0;
        } else {
            snprintf(sDotParamStr, sizeof(sDotParamStr), "????");
        }
        return sDotParamStr;
    }
    char *str = OP_TO_STR[op->op];
    if (*str) {
        return str;
    }
    if (op_is_register(op->op)) {
        snprintf(str, sizeof(*OP_TO_STR), "v_REG_%02X", op->op - CC_EXT_REGISTER_FIRST);
        return str;
    }
    if (op_is_constant(op->op)) {
        snprintf(str, sizeof(*OP_TO_STR), "v_CONST_%02X", op->op - CC_EXT_CONST_FIRST);
        return str;
    }
    return str;
}

//
// Operation result
//

#define CC_EXT_MAX_ARGS 5

typedef struct Result {
    Op *op;
    struct Result *args[CC_EXT_MAX_ARGS];
} Result;

static Result *result_new(struct GrowingArray *results, Op *op, Result **args) {
    Result *new_result = growing_array_alloc(results, sizeof(Result));
    new_result->op = op;
    if (args) {
        for (int i = 0; i < CC_EXT_MAX_ARGS; ++i) {
            new_result->args[i] = args[i];
        }
    }
    return new_result;
}

static inline Result *result_get(struct GrowingArray *results, uint32_t index) {
    return (Result *) results->buffer[index];
}

static bool result_get_range_internal(struct GrowingArray *ops, const Result *result, uint32_t *from, uint32_t *to) {
    if (result) {
        int op_index = op_get_index(ops, result->op);
        if (op_index >= 0) {
            *from = min(*from, (uint32_t) op_index);
            *to = max(*to, (uint32_t) op_index);
            for (uint8_t i = 0; i < CC_EXT_MAX_ARGS; ++i) {
                if (!result_get_range_internal(ops, result->args[i], from, to)) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    return true;
}

static bool result_get_range(struct GrowingArray *ops, const Result *result, uint32_t *from, uint32_t *to) {
    if (!result) { return false; }
    *from = ops->count;
    *to = 0;
    return result_get_range_internal(ops, result, from, to);
}

void gfx_cc_ext_optimize(struct GrowingArray *ops, struct GrowingArray *constants);

#endif
