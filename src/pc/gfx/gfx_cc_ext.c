#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "gfx_cc_ext.h"
#include "gfx_cc_ext_types.h"

#include "data/dynos_cmap.cpp.h"

static void *sExtCombinerCache = NULL;

static_assert(CC_EXT_CONST_LAST < CC_EXT_SPECIAL);

#define CC_EXT_OPERATOR(name, args, ...) static_assert(args <= CC_EXT_MAX_ARGS);
#define CC_EXT_FUNCTION(name, args)      static_assert(args <= CC_EXT_MAX_ARGS);
#include "gfx_cc_ext_operators.inl"
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_OPERATOR
#undef CC_EXT_FUNCTION

#define COLORED_PRINT 1

#if COLORED_PRINT
#define COLOR_RED       "\e[0;31m"
#define COLOR_GREEN     "\e[0;32m"
#define COLOR_YELLOW    "\e[0;33m"
#define COLOR_BLUE      "\e[0;34m"
#define COLOR_MAGENTA   "\e[0;35m"
#define COLOR_CYAN      "\e[0;36m"
#define COLOR_RESET     "\e[0m"
#else
#define COLOR_RED       ""
#define COLOR_GREEN     ""
#define COLOR_YELLOW    ""
#define COLOR_BLUE      ""
#define COLOR_MAGENTA   ""
#define COLOR_CYAN      ""
#define COLOR_RESET     ""
#endif

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

void gfx_cc_ext_print_ops(struct GrowingArray *ops, struct GrowingArray *constants) {
    growing_array_for_each_(ops, const Op, op) {
        if (op->is_dot_param) {
            printf(COLOR_YELLOW);
        } else if (op_is_constant(op->op)) {
            printf(COLOR_CYAN);
        } else if (op_is_value(op->op)) {
            printf(COLOR_BLUE);
        } else if (op_is_special(op->op)) {
            printf(COLOR_MAGENTA);
        } else if (op_is_operator(op->op)) {
            printf(COLOR_GREEN);
        }
        printf("%s ", op_to_string(op, constants));
        printf(COLOR_RESET);
    }
    printf("\n");
    for (uint32_t i = 0; i < constants->count; ++i) {
        const Constant *constant = constant_get(constants, i);
        printf("v_CONST_%02X: %f %f %f %f\n", i, constant->x, constant->y, constant->z, constant->w);
    }
    printf("\n");
}

//
// Infix to postfix
//

static inline void stack_push(struct GrowingArray *stack, Op *op) {
    op_push(stack, op->op, op->is_dot_param, op->token);
}

static inline Op *stack_top(struct GrowingArray *stack) {
    return op_get(stack, stack->count - 1);
}

static inline Op *stack_pop(struct GrowingArray *stack) {
    Op *op = stack_top(stack);
    stack->count--;
    return op;
}

static struct GrowingArray *gfx_cc_ext_generate_postfix_ops(struct GrowingArray *ops, char *error_msg, size_t msg_len, const char *str) {
    struct GrowingArray *res   = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *stack = growing_array_init(NULL, 16, malloc, free);
    bool ok = false;

    // from right to left
    for (int i = ops->count - 1; i >= 0; --i) {
        Op *op = op_get(ops, i);

        // value
        if (op_is_value(op->op)) {
            op_push(res, op->op, op->is_dot_param, op->token);
            continue;
        }

        // right parenthesis, beginning of scope
        if (op->op == CC_EXT_CLOSE) {
            stack_push(stack, op);
            continue;
        }

        // left parenthesis or comma, end of scope or function argument
        if (op->op == CC_EXT_OPEN || op->op == CC_EXT_COMMA) {
            while (stack->count > 0 && stack_top(stack)->op != CC_EXT_CLOSE) {
                Op *top = stack_pop(stack);
                op_push(res, top->op, top->is_dot_param, top->token);
            }

            // go to next function argument
            if (op->op == CC_EXT_COMMA) {
                continue;
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
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
                }
                goto cleanup;
            } else {
                stack_pop(stack);
            }
            continue;
        }

        // operator/function
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
                op_push(res, top->op, top->is_dot_param, top->token);
            }
            stack_push(stack, op);
            continue;
        }

        // invalid
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
            gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
        }
        goto cleanup;
    }

    // pop remaining ops
    while (stack->count > 0) {
        Op *op = stack_pop(stack);
        op_push(res, op->op, op->is_dot_param, op->token);
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
                            "VALIDATION FAILED: Constant %u at position %llu is not defined, there is only %u constants loaded.\n",
                            constant_index, op->token->pos, constants->count
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                    } else {
                        snprintf(error_msg, msg_len,
                            "VALIDATION FAILED: Constant %u is not defined, there is only %u constants loaded.\n",
                            constant_index, constants->count
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
                    }
                    return false;
                }
            }

            // check stack size before push
            if (stack_size == CC_EXT_STACK_SIZE) {
                if (op->token) {
                    snprintf(error_msg, msg_len,
                        "VALIDATION FAILED: Stack is full, cannot push value at position %llu: `%s`\n",
                        op->token->pos, op->token->str
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                } else {
                    snprintf(error_msg, msg_len,
                        "VALIDATION FAILED: Stack is full, cannot push more values."
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
                }
                return false;
            }

            stack[stack_size++] = op;
            continue;
        }

        // operators
        if (op_is_operator(op->op)) {
            uint8_t args = op_get_num_args(op->op);

            // check if there is enough values in stack
            if (stack_size < args) {
                if (op->token) {
                    snprintf(error_msg, msg_len,
                        "VALIDATION FAILED: Not enough values in stack for %s at position %llu: `%s` (%llu values, should be at least %u)\n",
                        op_is_function(op->op) ? "function" : "operator", op->token->pos, op->token->str, stack_size, args
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
                } else {
                    snprintf(error_msg, msg_len,
                        "VALIDATION FAILED: Not enough values in stack for %s: `%s` (%llu values, should be at least %u)\n",
                        op_is_function(op->op) ? "function" : "operator", op_to_string(op, constants), stack_size, args
                    );
                    gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
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
                            "VALIDATION FAILED: Argument %u of %s `%s` at position %llu is not a value: `%s`\n",
                            k, op_is_function(op->op) ? "function" : "operator", op->token->str, op->token->pos, arg->token ? arg->token->str : op_to_string(arg, constants)
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, arg->token ? arg->token->pos : op->token->pos, arg->token ? arg->token->len : op->token->len, 0);
                    } else {
                        snprintf(error_msg, msg_len,
                            "VALIDATION FAILED: Argument %u of %s `%s` is not a value: `%s`\n",
                            k, op_is_function(op->op) ? "function" : "operator", op_to_string(op, constants), arg->token ? arg->token->str : op_to_string(arg, constants)
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
                    }
                    return false;
                }

                // arg 2 of dot operator must be a dot param
                if (op->op == CC_EXT_DOT && k == 1) {
                    if (!arg->is_dot_param) {
                        if (op->token) {
                            snprintf(error_msg, msg_len,
                                "VALIDATION FAILED: Argument %u of dot operator `.` at position %llu is not a dot param: `%s`\n",
                                k + 1, op->token->pos, arg->token ? arg->token->str : op_to_string(arg, constants)
                            );
                            gfx_cc_ext_print_error_location(error_msg, msg_len, str, arg->token ? arg->token->pos : op->token->pos, arg->token ? arg->token->len : op->token->len, 0);
                        } else {
                            snprintf(error_msg, msg_len,
                                "VALIDATION FAILED: Argument %u of dot operator `.` is not a dot param: `%s`\n",
                                k + 1, arg->token ? arg->token->str : op_to_string(arg, constants)
                            );
                            gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
                        }
                        return false;
                    }
                }

                // otherwise, arg must not be a dot param
                else if (arg->is_dot_param) {
                    if (op->token) {
                        snprintf(error_msg, msg_len,
                            "VALIDATION FAILED: Argument %u of %s `%s` at position %llu must not be a dot param: `%s`\n",
                            k + 1, op_is_function(op->op) ? "function" : "operator", op->token->str, op->token->pos, arg->token ? arg->token->str : op_to_string(arg, constants)
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, arg->token ? arg->token->pos : op->token->pos, arg->token ? arg->token->len : op->token->len, 0);
                    } else {
                        snprintf(error_msg, msg_len,
                            "VALIDATION FAILED: Argument %u of %s `%s` must not be a dot param: `%s`\n",
                            k + 1, op_is_function(op->op) ? "function" : "operator", op_to_string(op, constants), arg->token ? arg->token->str : op_to_string(arg, constants)
                        );
                        gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
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
                "VALIDATION FAILED: Invalid op 0x%02X from token at position %llu: `%s`\n",
                op->op, op->token->pos, op->token->str
            );
            gfx_cc_ext_print_error_location(error_msg, msg_len, str, op->token->pos, op->token->len, 0);
        } else {
            snprintf(error_msg, msg_len,
                "VALIDATION FAILED: Invalid op 0x%02X.",
                op->op
            );
            gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
        }
        return false;
    }

    // empty combiner
    if (stack_size == 0) {
        snprintf(error_msg, msg_len,
            "VALIDATION FAILED: Combiner is empty, no result in stack."
        );
        gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
        return false;
    }

    // too much values in stack
    if (stack_size > 1) {
        snprintf(error_msg, msg_len,
            "VALIDATION FAILED: Combiner is ill-formed, too much values (%llu) remain in stack after processing.",
            stack_size
        );
        gfx_cc_ext_print_error_location(error_msg, msg_len, str, 0, strlen(str), 0);
        return false;
    }

    return true;
}

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

static const char *gfx_cc_ext_parse_combiner(const char *str, struct ExtendedCombiner *ec) {
    static char sErrorMsg[1024];

    struct GrowingArray *tokens    = growing_array_init(NULL, 16, malloc, token_free);
    struct GrowingArray *ops       = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *constants = growing_array_init(NULL, 16, malloc, free);
    struct GrowingArray *postfix   = NULL;
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

    // turn tokens into ops
    Token *scope = NULL;
    growing_array_for_each_(tokens, Token, token) {
        uint8_t prev = (ops->count > 0 ? op_get(ops, ops->count - 1)->op : CC_EXT_COMMA);

        // manually checked symbols
        // includes unary plus and minus, parentheses and comma ops

        if (token_is("+")) {
            if (op_can_be_value(prev)) {
                op_push(ops, CC_EXT_ADD, false, token);
            } else {
                // ignore unary plus
            }
            continue;
        }

        if (token_is("-")) {
            if (op_can_be_value(prev)) {
                op_push(ops, CC_EXT_SUB, false, token);
            } else {
                op_push(ops, CC_EXT_NEG, false, token);
            }
            continue;
        }

        if (token_is("(")) {
            token->prev = scope;
            scope = token;
            if (op_can_be_operator(prev)) {
                op_push(ops, CC_EXT_OPEN, false, token);
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
            op_push(ops, CC_EXT_CLOSE, false, token);
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

            op_push(ops, CC_EXT_COMMA, false, token);
            continue;
        }

        // dot operator param
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

            float4 dot_param = {0};
            for (size_t i = 0; i < 4; ++i) {
                char c = (
                    i < len ? token->str[i] : (
                    len == 1 ? token->str[0] : (
                    len == 3 ? '1' : '0'
                )));
                uint8_t k = 0;
                switch (c) {
                    case '0':           k = CC_EXT_DOT_PARAM_0; break;
                    case '1':           k = CC_EXT_DOT_PARAM_1; break;
                    case 'r': case 'R': k = CC_EXT_DOT_PARAM_R; break;
                    case 'g': case 'G': k = CC_EXT_DOT_PARAM_G; break;
                    case 'b': case 'B': k = CC_EXT_DOT_PARAM_B; break;
                    case 'a': case 'A': k = CC_EXT_DOT_PARAM_A; break;
                    default:
                        snprintf(sErrorMsg, sizeof(sErrorMsg),
                            "Operator `.` parameter `%s` at position %llu has invalid character:  %c  (%d, 0x%02X) (must be one of \"rgba01\").\n",
                            token->str, token->pos, c, c, c
                        );
                        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, i);
                        goto cleanup;
                }
                dot_param[i] = k;
            }

            if (!op_push_constant(ops, constants, true, dot_param, token)) {
                snprintf(sErrorMsg, sizeof(sErrorMsg),
                    "Max number of constants reached: %u\n",
                    CC_EXT_MAX_CONSTANTS
                );
                gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, token->pos, token->len, 0);
                goto cleanup;
            }
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

            float4 constant4 = { constant, constant, constant, constant };
            if (!op_push_constant(ops, constants, false, constant4, token)) {
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

    // convert to postfix
    postfix = gfx_cc_ext_generate_postfix_ops(ops, sErrorMsg, sizeof(sErrorMsg), str);
    if (!postfix) {
        goto cleanup;
    }

    // validate
    if (!gfx_cc_ext_validate(postfix, constants, sErrorMsg, sizeof(sErrorMsg), str)) {
        goto cleanup;
    }

#if !GFX_CC_EXT_DISABLE_OPTIMIZATION
    // optimize
    gfx_cc_ext_optimize(postfix, constants);

    // validate again
    if (!gfx_cc_ext_validate(postfix, constants, sErrorMsg, sizeof(sErrorMsg), str)) {
        goto cleanup;
    }
#endif

#if GFX_CC_EXT_PRINT_OPS_AFTER_PARSING
    printf("%s\n", str);
    gfx_cc_ext_print_ops(postfix, constants);
#endif

    // check sizes
    if (postfix->count > CC_EXT_MAX_OPS) {
        snprintf(sErrorMsg, sizeof(sErrorMsg),
            "Max number of ops exceeded: %u > %u\n",
            postfix->count, CC_EXT_MAX_OPS
        );
        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, 0, len, 0);
        goto cleanup;
    }
    if (constants->count > CC_EXT_MAX_CONSTANTS) {
        snprintf(sErrorMsg, sizeof(sErrorMsg),
            "Max number of constants exceeded: %u > %u\n",
            constants->count, CC_EXT_MAX_CONSTANTS
        );
        gfx_cc_ext_print_error_location(sErrorMsg, sizeof(sErrorMsg), str, 0, len, 0);
        goto cleanup;
    }

    // fill struct
    memset(ec, 0, sizeof(*ec));
    ec->num_ops = postfix->count;
    for (uint32_t i = 0; i < postfix->count; ++i) {
        ec->ops[i] = op_get(postfix, i)->op;
    }
    ec->num_constants = constants->count;
    for (uint32_t i = 0; i < constants->count; ++i) {
        memcpy(ec->constants[i], constant_get(constants, i)->xyzw, sizeof(float4));
    }
    ok = true;

cleanup:
    growing_array_free(&tokens);
    growing_array_free(&ops);
    growing_array_free(&constants);
    growing_array_free(&postfix);
    return ok ? NULL : sErrorMsg;
}

const struct ExtendedCombiner *gfx_cc_ext_get_combiner(const char *str, const char **error_msg) {
    static struct ExtendedCombiner sNullExtCombiner;
    if (!str) { return NULL; }

    // Check cache
    size_t len = strlen(str);
    struct ExtendedCombiner *ec = hmap_data_get(sExtCombinerCache, str, len);
    if (ec) {
        return ec == &sNullExtCombiner ? NULL : ec;
    }

    // Allocate cache
    if (!sExtCombinerCache) {
        sExtCombinerCache = hmap_data_create();
        if (!sExtCombinerCache) {
            sys_fatal("Cannot allocate memory to create the extended combiner cache!");
        }
    }

    // Allocate combiner
    ec = malloc(sizeof(struct ExtendedCombiner));
    if (!ec) {
        if (error_msg) { *error_msg = "Cannot allocate memory to create an extended combiner!"; }
        return NULL;
    }

    // Parse combiner
    const char *err = gfx_cc_ext_parse_combiner(str, ec);
    if (err) {
        if (error_msg) { *error_msg = err; }
        hmap_data_put(sExtCombinerCache, str, len, &sNullExtCombiner);
        free(ec);
        return NULL;
    }

    hmap_data_put(sExtCombinerCache, str, len, ec);
    return ec;
}

void gfx_cc_ext_clear_combiner_cache() {
    void *ec = hmap_data_begin(sExtCombinerCache);
    while (ec) {
        free(ec);
        ec = hmap_data_next(sExtCombinerCache);
    }
    hmap_data_destroy(sExtCombinerCache);
    sExtCombinerCache = NULL;
}
