#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "engine/math_util.h"
#include "game/memory.h"

#define DEFINE_CC_EXT_TYPES
#include "gfx_cc_ext.h"

#define PRINT_OPTIMIZATION 1
#define PRINT_OPS 1

#if PRINT_OPS
extern void gfx_cc_ext_print_ops(struct GrowingArray *ops, struct GrowingArray *constants);
#else
#define gfx_cc_ext_print_ops(...)
#endif

#if PRINT_OPTIMIZATION
#define print_optimization(...) { \
    printf("OPTIMIZATION: " __VA_ARGS__); \
    gfx_cc_ext_print_ops(ops, constants); \
}
#else
#define print_optimization(...)
#endif

#define Op_(op, is_dot_param, token) \
    ((Op *)(Op []){{ op, is_dot_param, token }})

//
// Operation result
//

typedef struct Result {
    Op *op;
    struct Result *args[4];
} Result;

static Result *result_new(struct GrowingArray *results, Op *op, Result *arg1, Result *arg2, Result *arg3, Result *arg4) {
    Result *new_result = growing_array_alloc(results, sizeof(Result));
    new_result->op = op;
    new_result->args[0] = arg1;
    new_result->args[1] = arg2;
    new_result->args[2] = arg3;
    new_result->args[3] = arg4;
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
            for (uint8_t i = 0; i < 4; ++i) {
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

//
// Constants
//

static void gfx_cc_ext_reindex_constants(struct GrowingArray *ops, struct GrowingArray *constants) {

    // detect unused constants
    bool used_constants[CC_EXT_MAX_CONSTANTS] = {0};
    growing_array_for_each_(ops, const Op, op) {
        if (op_is_constant(op->op)) {
            used_constants[op->op - CC_EXT_CONST_FIRST] = true;
        }
    }

    // pop unused constants
    uint8_t reindexed_constants[CC_EXT_MAX_CONSTANTS] = {0};
    uint8_t constants_count = constants->count;
    for (uint8_t i = 0, k = 0; i < constants_count; ++i) {
        if (used_constants[i]) {
            reindexed_constants[i] = k++;
        } else {
            reindexed_constants[i] = 0xFF;
            growing_array_pop_index(constants, k);
        }
    }

    // reindex ops
    growing_array_for_each_(ops, Op, op) {
        if (op_is_constant(op->op)) {
            uint8_t constant_index = op->op - CC_EXT_CONST_FIRST;
            op->op = CC_EXT_CONST_FIRST + reindexed_constants[constant_index];
        }
    }
}

static bool gfx_cc_ext_optimize_constant(struct GrowingArray *ops, struct GrowingArray *constants, const Op *op) {
    if (op_is_constant(op->op)) {
        const Constant *constant = constant_get(constants, op->op - CC_EXT_CONST_FIRST);
        int value_op = constant_get_builtin_value(constant->xyzw);
        if (value_op >= 0) {

            // replace all ops that use this constant by the builtin value
            uint8_t constant_op = op->op;
            growing_array_for_each_(ops, Op, _op) {
                if (_op->op == constant_op) {
                    _op->op = value_op;
                }
            }

            // remove unused constant and reindex
            gfx_cc_ext_reindex_constants(ops, constants);

            print_optimization("Replaced `%s` by `%s`.\n",
                op_to_string(Op_(constant_op, op->is_dot_param, op->token), constants),
                op_to_string(Op_(value_op, op->is_dot_param, op->token), constants)
            );
            return true;
        }
    }
    return false;
}

//
// Operations
//

#define define_argf(n)      float4 arg##n##f = {0};
#define arg_to_float4(n)    && op_to_float4(arg##n->op->op, constants, arg##n##f)
#define remove_arg(n)       gfx_cc_ext_remove_result(ops, arg##n);
#define print_argf(n)       arg##n##f[0], arg##n##f[1], arg##n##f[2], arg##n##f[3],

#define check_constant_operation(N, format, ...) { \
    REPEAT(define_argf, N); \
    if (1 \
        REPEAT(arg_to_float4, N) \
        ) { \
        /* compute new constant */ \
        float4 xyzw = {0}; \
        for (int i = 0; i < 4; ++i) { \
            __VA_ARGS__; \
        } \
        /* push new constant */ \
        /* don't optimize if there is no space left for constant */ \
        if (!op_push_constant(ops, constants, false, xyzw, op->token)) { \
            return false; \
        } \
        /* insert constant, remove operation */ \
        gfx_cc_ext_replace_op(ops, op); \
        REPEAT(remove_arg, N); \
        gfx_cc_ext_reindex_constants(ops, constants); \
        print_optimization("Replaced `" format "` by `{%f, %f, %f, %f}`.\n", \
            REPEAT(print_argf, N) \
            xyzw[0], xyzw[1], xyzw[2], xyzw[3] \
        ); \
        return true; \
    } \
}

#define check_constant_operation_1(format, ...) \
    check_constant_operation(1, format, __VA_ARGS__)

#define check_constant_operation_2(format, ...) \
    check_constant_operation(2, format, __VA_ARGS__)

#define check_constant_operation_3(format, ...) \
    check_constant_operation(3, format, __VA_ARGS__)

#define check_constant_operation_4(format, ...) \
    check_constant_operation(4, format, __VA_ARGS__)

static inline void gfx_cc_ext_replace_op(struct GrowingArray *ops, const Op *op) {
    int index = op_get_index(ops, op);
    growing_array_swap_and_pop_index(ops, index);
}

static inline void gfx_cc_ext_remove_op(struct GrowingArray *ops, const Op *op) {
    int index = op_get_index(ops, op);
    growing_array_pop_index(ops, index);
}

static inline void gfx_cc_ext_move_result(struct GrowingArray *ops, const Result *result, uint32_t index) {
    uint32_t from, to;
    if (result_get_range(ops, result, &from, &to)) {
        growing_array_move(ops, from, index, to - from + 1);
    }
}

static inline void gfx_cc_ext_remove_result(struct GrowingArray *ops, const Result *result) {
    uint32_t from, to;
    if (result_get_range(ops, result, &from, &to)) {
        growing_array_pop_range(ops, from, to - from + 1);
    }
}

static bool gfx_cc_ext_optimize_DOT(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    float4 arg2f;
    if (op_to_float4(arg2->op->op, constants, arg2f)) {

        // X.rgba == X
        static const float4 DOT_PARAM_RGBA = { CC_EXT_DOT_PARAM_R, CC_EXT_DOT_PARAM_G, CC_EXT_DOT_PARAM_B, CC_EXT_DOT_PARAM_A };
        if (float4_eq(arg2f, DOT_PARAM_RGBA)) {
            UNUSED const char *dot_param_str = op_to_string(arg2->op, constants);

            // remove dot op and arg 2
            gfx_cc_ext_remove_op(ops, arg2->op);
            gfx_cc_ext_remove_op(ops, op);
            gfx_cc_ext_reindex_constants(ops, constants);

            print_optimization("Replaced `X.%s` by `X`.\n",
                dot_param_str
            );
            return true;
        }

        // X.0|1 == C
        if ((arg2f[0] == 0 || arg2f[0] == 1) &&
            (arg2f[1] == 0 || arg2f[1] == 1) &&
            (arg2f[2] == 0 || arg2f[2] == 1) &&
            (arg2f[3] == 0 || arg2f[3] == 1)) {
            UNUSED const char *dot_param_str = op_to_string(arg2->op, constants);

            // remove operation, turn dot param into constant
            arg2->op->is_dot_param = false;
            gfx_cc_ext_remove_result(ops, arg1);
            gfx_cc_ext_remove_op(ops, op);
            gfx_cc_ext_reindex_constants(ops, constants);

            print_optimization("Replaced `X.%s` by `%s`.\n",
                dot_param_str,
                op_to_string(arg2->op, constants)
            );
            return true;
        }
    }

    // C.abcd == C{x=a, y=b, z=c, w=d}
    check_constant_operation_2(
        "{%f, %f, %f, %f}.{%f, %f, %f, %f}",
        switch ((int) arg2f[i]) {
            case CC_EXT_DOT_PARAM_0: xyzw[i] = 0; break;
            case CC_EXT_DOT_PARAM_1: xyzw[i] = 1; break;
            case CC_EXT_DOT_PARAM_R: xyzw[i] = arg1f[0]; break;
            case CC_EXT_DOT_PARAM_G: xyzw[i] = arg1f[1]; break;
            case CC_EXT_DOT_PARAM_B: xyzw[i] = arg1f[2]; break;
            case CC_EXT_DOT_PARAM_A: xyzw[i] = arg1f[3]; break;
        }
    )

    return false;
}

static bool gfx_cc_ext_optimize_NEG(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // --X == X
    const Op *next_op = op_get(ops, op_get_index(ops, op) + 1);
    if (next_op && next_op->op == CC_EXT_NEG) {

        // remove both ops
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_op(ops, next_op);

        print_optimization("Replaced `--X` by `X`.\n");
        return true;
    }

    // -0 == 0
    if (arg1->op->op == CC_EXT_0) {

        // remove op
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `-0` by `0`.\n");
        return true;
    }

    // -C1 == C
    check_constant_operation_1(
        "-{%f, %f, %f, %f}",
        xyzw[i] = -arg1f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_NOT(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // !!!X == !X
    const Op *next_op_1 = op_get(ops, op_get_index(ops, op) + 1);
    const Op *next_op_2 = op_get(ops, op_get_index(ops, op) + 2);
    if (next_op_1 && next_op_1->op == CC_EXT_NOT &&
        next_op_2 && next_op_2->op == CC_EXT_NOT) {

        // remove both next ops
        gfx_cc_ext_remove_op(ops, next_op_1);
        gfx_cc_ext_remove_op(ops, next_op_2);

        print_optimization("Replaced `!!!X` by `!X`.\n");
        return true;
    }

    // !(X > Y) == X <= Y
    if (arg1->op->op == CC_EXT_GT) {

        // change arg op, remove op
        arg1->op->op = CC_EXT_LEQ;
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `!(X > Y)` by `X <= Y`.\n");
        return true;
    }

    // !(X >= Y) == X < Y
    if (arg1->op->op == CC_EXT_GEQ) {

        // change arg op, remove op
        arg1->op->op = CC_EXT_LT;
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `!(X >= Y)` by `X < Y`.\n");
        return true;
    }

    // !(X < Y) == X >= Y
    if (arg1->op->op == CC_EXT_LT) {

        // change arg op, remove op
        arg1->op->op = CC_EXT_GEQ;
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `!(X < Y)` by `X >= Y`.\n");
        return true;
    }

    // !(X <= Y) == X > Y
    if (arg1->op->op == CC_EXT_LEQ) {

        // change arg op, remove op
        arg1->op->op = CC_EXT_GT;
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `!(X <= Y)` by `X > Y`.\n");
        return true;
    }

    // !(X == Y) == X != Y
    if (arg1->op->op == CC_EXT_EQ) {

        // change arg op, remove op
        arg1->op->op = CC_EXT_NEQ;
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `!(X == Y)` by `X != Y`.\n");
        return true;
    }

    // !(X != Y) == X == Y
    if (arg1->op->op == CC_EXT_NEQ) {

        // change arg op, remove op
        arg1->op->op = CC_EXT_EQ;
        gfx_cc_ext_remove_op(ops, op);

        print_optimization("Replaced `!(X != Y)` by `X == Y`.\n");
        return true;
    }

    // !C1 == C
    check_constant_operation_1(
        "!{%f, %f, %f, %f}",
        xyzw[i] = (float) (arg1f[i] == 0 ? 1 : 0);
    );

    return false;
}

static bool gfx_cc_ext_optimize_POW(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // X ^ 0 == 1
    if (arg2->op->op == CC_EXT_0) {

        // replace operation by constant 1
        op->op = CC_EXT_1;
        gfx_cc_ext_remove_result(ops, arg1);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `X ^ 0` by `1`.\n");
        return true;
    }

    // X ^ 1 == X
    if (arg2->op->op == CC_EXT_1) {

        // remove operation, keep arg1
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `X ^ 1` by `X`.\n");
        return true;
    }

    // C1 ^ C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} ^ {%f, %f, %f, %f}",
        xyzw[i] = powf(arg1f[i], arg2f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_MUL(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // X * 0 == 0 * X == 0
    if (arg1->op->op == CC_EXT_0 ||
        arg2->op->op == CC_EXT_0) {

        // replace operation by constant 0
        op->op = CC_EXT_0;
        gfx_cc_ext_remove_result(ops, arg1);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        if (arg1->op->op == CC_EXT_0) {
            print_optimization("Replaced `0 * X` by `0`.\n");
        } else {
            print_optimization("Replaced `X * 0` by `0`.\n");
        }
        return true;
    }
    
    // X * 1 == 1 * X == X
    if (arg1->op->op == CC_EXT_1 ||
        arg2->op->op == CC_EXT_1) {

        // remove operation, keep arg1 or arg2
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg1->op->op == CC_EXT_1 ? arg1 : arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        if (arg1->op->op == CC_EXT_1) {
            print_optimization("Replaced `1 * X` by `X`.\n");
        } else {
            print_optimization("Replaced `X * 1` by `X`.\n");
        }
        return true;
    }

    // -X * -Y == X * Y
    if (arg1->op->op == CC_EXT_NEG ||
        arg2->op->op == CC_EXT_NEG) {

        bool arg1_is_neg = arg1->op->op == CC_EXT_NEG;
        Result *X = arg1_is_neg ? arg1 : arg2;
        Result *Y = arg1_is_neg ? arg2 : arg1;

        float4 argf;
        if (op_to_float4(Y->op->op, constants, argf)) {

            // negate constant
            float4 nargf = { -argf[0], -argf[1], -argf[2], -argf[3] };

            // push new constant
            // don't optimize if there is no space left for constant
            if (!op_push_constant(ops, constants, false, nargf, op->token)) {
                return false;
            }

            // replace constant and remove unary minus
            gfx_cc_ext_replace_op(ops, Y->op);
            gfx_cc_ext_remove_op(ops, X->op);
            gfx_cc_ext_reindex_constants(ops, constants);

            if (arg1_is_neg) {
                print_optimization("Replaced `-X * {%f, %f, %f, %f}` by `X * {%f, %f, %f, %f}`.\n",
                    argf[0], argf[1], argf[2], argf[3],
                    nargf[0], nargf[1], nargf[2], nargf[3]
                );
            } else {
                print_optimization("Replaced `{%f, %f, %f, %f} * -X` by `{%f, %f, %f, %f} * X`.\n",
                    argf[0], argf[1], argf[2], argf[3],
                    nargf[0], nargf[1], nargf[2], nargf[3]
                );
            }
            return true;
        }
        if (Y->op->op == CC_EXT_NEG) {

            // remove both unary minus
            gfx_cc_ext_remove_op(ops, X->op);
            gfx_cc_ext_remove_op(ops, Y->op);

            print_optimization("Replaced `-X * -Y` by `X * Y`.\n");
            return true;
        }
    }

    // C1 * C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} * {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] * arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_DIV(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // X / 1 == X
    if (arg2->op->op == CC_EXT_1) {

        // remove operation, keep arg1
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `X / 1` by `X`.\n");
        return true;
    }

    // 0 / X == 0
    if (arg1->op->op == CC_EXT_0) {

        // replace operation by constant 0
        op->op = CC_EXT_0;
        gfx_cc_ext_remove_result(ops, arg1);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `0 / X` by `0`.\n");
        return true;
    }

    // -X / -Y == X / Y
    if (arg1->op->op == CC_EXT_NEG ||
        arg2->op->op == CC_EXT_NEG) {

        bool arg1_is_neg = arg1->op->op == CC_EXT_NEG;
        Result *X = arg1_is_neg ? arg1 : arg2;
        Result *Y = arg1_is_neg ? arg2 : arg1;

        float4 argf;
        if (op_to_float4(Y->op->op, constants, argf)) {

            // negate constant
            float4 nargf = { -argf[0], -argf[1], -argf[2], -argf[3] };

            // push new constant
            // don't optimize if there is no space left for constant
            if (!op_push_constant(ops, constants, false, nargf, op->token)) {
                return false;
            }

            // replace constant and remove unary minus
            gfx_cc_ext_replace_op(ops, Y->op);
            gfx_cc_ext_remove_op(ops, X->op);
            gfx_cc_ext_reindex_constants(ops, constants);

            if (arg1_is_neg) {
                print_optimization("Replaced `-X / {%f, %f, %f, %f}` by `X / {%f, %f, %f, %f}`.\n",
                    argf[0], argf[1], argf[2], argf[3],
                    nargf[0], nargf[1], nargf[2], nargf[3]
                );
            } else {
                print_optimization("Replaced `{%f, %f, %f, %f} / -X` by `{%f, %f, %f, %f} / X`.\n",
                    argf[0], argf[1], argf[2], argf[3],
                    nargf[0], nargf[1], nargf[2], nargf[3]
                );
            }
            return true;
        }
        if (Y->op->op == CC_EXT_NEG) {

            // remove both unary minus
            gfx_cc_ext_remove_op(ops, X->op);
            gfx_cc_ext_remove_op(ops, Y->op);

            print_optimization("Replaced `-X / -Y` by `X / Y`.\n");
            return true;
        }
    }

    // 1 / sqrt(X) == rsqrt(X)
    // 1 / rsqrt(X) == sqrt(X)
    if (arg1->op->op == CC_EXT_1 &&
        (arg2->op->op == CC_EXT_SQRT || arg2->op->op == CC_EXT_RSQRT)) {

        bool arg2_is_sqrt = arg2->op->op == CC_EXT_SQRT;

        // change arg op, remove op and 1
        arg2->op->op = arg2_is_sqrt ? CC_EXT_RSQRT : CC_EXT_SQRT;
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg1);

        if (arg2_is_sqrt) {
            print_optimization("Replaced `1 / sqrt(X)` by `rsqrt(X)`.\n");
        } else {
            print_optimization("Replaced `1 / rsqrt(X)` by `sqrt(X)`.\n");
        }
        return true;
    }

    // C1 / C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} / {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] / arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_MOD(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // 0 % X == 0
    if (arg1->op->op == CC_EXT_0) {

        // replace operation by constant 0
        op->op = CC_EXT_0;
        gfx_cc_ext_remove_result(ops, arg1);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `0 %% X` by `0`.\n");
        return true;
    }

    // C1 % C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} %% {%f, %f, %f, %f}",
        xyzw[i] = fmodf(arg1f[i], arg2f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_ADD(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // X + 0 == 0 + X == X
    if (arg1->op->op == CC_EXT_0 ||
        arg2->op->op == CC_EXT_0) {

        // remove operation, keep arg1 or arg2
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg1->op->op == CC_EXT_0 ? arg1 : arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        if (arg1->op->op == CC_EXT_0) {
            print_optimization("Replaced `0 + X` by `X`.\n");
        } else {
            print_optimization("Replaced `X + 0` by `X`.\n");
        }
        return true;
    }

    // X + -Y == X - Y
    if (arg2->op->op == CC_EXT_NEG) {

        // change op and remove unary minus
        op->op = CC_EXT_SUB;
        gfx_cc_ext_remove_op(ops, arg2->op);

        print_optimization("Replaced `X + -Y` by `X - Y`.\n");
        return true;
    }

    // -X + Y == Y - X
    if (arg1->op->op == CC_EXT_NEG) {

        // change op, swap args and remove unary minus
        op->op = CC_EXT_SUB;
        gfx_cc_ext_move_result(ops, arg1, op_get_index(ops, arg2->op));
        gfx_cc_ext_remove_op(ops, arg1->op);

        print_optimization("Replaced `-X + Y` by `Y - X`.\n");
        return true;
    }

    // C1 + C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} + {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] + arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_SUB(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // X - 0 == X
    if (arg2->op->op == CC_EXT_0) {

        // remove operation, keep arg1
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `X - 0` by `X`.\n");
        return true;
    }

    // 0 - X == -X
    if (arg1->op->op == CC_EXT_0) {

        // change op to unary minus, keep arg2
        op->op = CC_EXT_NEG;
        gfx_cc_ext_remove_result(ops, arg1);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `0 - X` by `-X`.\n");
        return true;
    }

    // X - -Y == X + Y
    if (arg2->op->op == CC_EXT_NEG) {

        // change op and remove unary minus
        op->op = CC_EXT_ADD;
        gfx_cc_ext_remove_op(ops, arg2->op);

        print_optimization("Replaced `X - -Y` by `X + Y`.\n");
        return true;
    }

    // C1 - C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} - {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] - arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_LT(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // -X < -Y == X > Y
    if (arg1->op->op == CC_EXT_NEG &&
        arg2->op->op == CC_EXT_NEG) {

        // change op and remove both unary minus
        op->op = CC_EXT_GT;
        gfx_cc_ext_remove_op(ops, arg1->op);
        gfx_cc_ext_remove_op(ops, arg2->op);

        print_optimization("Replaced `-X < -Y` by `X > Y`.\n");
        return true;
    }

    // C1 < C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} < {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] < arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_LEQ(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // -X <= -Y == X >= Y
    if (arg1->op->op == CC_EXT_NEG &&
        arg2->op->op == CC_EXT_NEG) {

        // change op and remove both unary minus
        op->op = CC_EXT_GEQ;
        gfx_cc_ext_remove_op(ops, arg1->op);
        gfx_cc_ext_remove_op(ops, arg2->op);

        print_optimization("Replaced `-X <= -Y` by `X >= Y`.\n");
        return true;
    }

    // C1 <= C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} <= {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] <= arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_GT(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // -X > -Y == X < Y
    if (arg1->op->op == CC_EXT_NEG &&
        arg2->op->op == CC_EXT_NEG) {

        // change op and remove both unary minus
        op->op = CC_EXT_LT;
        gfx_cc_ext_remove_op(ops, arg1->op);
        gfx_cc_ext_remove_op(ops, arg2->op);

        print_optimization("Replaced `-X > -Y` by `X < Y`.\n");
        return true;
    }

    // C1 > C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} > {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] > arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_GEQ(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // -X >= -Y == X <= Y
    if (arg1->op->op == CC_EXT_NEG &&
        arg2->op->op == CC_EXT_NEG) {

        // change op and remove both unary minus
        op->op = CC_EXT_LEQ;
        gfx_cc_ext_remove_op(ops, arg1->op);
        gfx_cc_ext_remove_op(ops, arg2->op);

        print_optimization("Replaced `-X >= -Y` by `X <= Y`.\n");
        return true;
    }

    // C1 >= C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} >= {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] >= arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_EQ(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // C1 == C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} == {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] == arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_NEQ(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // C1 != C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} != {%f, %f, %f, %f}",
        xyzw[i] = arg1f[i] != arg2f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_AND(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // C1 && C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} && {%f, %f, %f, %f}",
        xyzw[i] = ((arg1f[i] != 0) && (arg2f[i] != 0) ? 1 : 0);
    );

    return false;
}

static bool gfx_cc_ext_optimize_OR(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // C1 || C2 == C
    check_constant_operation_2(
        "{%f, %f, %f, %f} || {%f, %f, %f, %f}",
        xyzw[i] = ((arg1f[i] != 0) || (arg2f[i] != 0) ? 1 : 0);
    );

    return false;
}

static bool gfx_cc_ext_optimize_VEC(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];
    Result *arg3 = stack[stack_size - 3];
    Result *arg4 = stack[stack_size - 4];

    // vec(C1, C2, C3, C4) == C{x=C1.x, y=C2.y, z=C3.z, w=C4.w}
    check_constant_operation_4(
        "vec({%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f})",
        xyzw[0] = arg1f[0];
        xyzw[1] = arg2f[1];
        xyzw[2] = arg3f[2];
        xyzw[3] = arg4f[3];
        break;
    );

    return false;
}

static bool gfx_cc_ext_optimize_LERP(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];
    Result *arg3 = stack[stack_size - 3];

    // lerp(X, Y, 0) == X
    if (arg3->op->op == CC_EXT_0) {

        // remove op, Y and 0
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_remove_result(ops, arg3);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `lerp(X, Y, 0)` by `X`.\n");
        return true;
    }

    // lerp(X, Y, 1) == Y
    if (arg3->op->op == CC_EXT_1) {

        // remove op, X and 1
        gfx_cc_ext_remove_op(ops, op);
        gfx_cc_ext_remove_result(ops, arg1);
        gfx_cc_ext_remove_result(ops, arg3);
        gfx_cc_ext_reindex_constants(ops, constants);

        print_optimization("Replaced `lerp(X, Y, 1)` by `Y`.\n");
        return true;
    }

    // lerp(C1, C2, C3) == C
    check_constant_operation_3(
        "lerp({%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f})",
        xyzw[i] = arg1f[i] * (1.f - arg3f[i]) + arg2f[i] * arg3f[i];
    );

    return false;
}

static bool gfx_cc_ext_optimize_INVLERP(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];
    Result *arg3 = stack[stack_size - 3];

    // invlerp(C1, C2, C3) == C
    check_constant_operation_3(
        "invlerp({%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f})",
        xyzw[i] = (arg3f[i] - arg1f[i]) / (arg2f[i] - arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_ABS(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // abs(C1) == C
    check_constant_operation_1(
        "abs({%f, %f, %f, %f})",
        xyzw[i] = fabsf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_MIN(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // min(C1, C2) == C
    check_constant_operation_2(
        "min({%f, %f, %f, %f}, {%f, %f, %f, %f})",
        xyzw[i] = min(arg1f[i], arg2f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_MAX(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];

    // max(C1, C2) == C
    check_constant_operation_2(
        "max({%f, %f, %f, %f}, {%f, %f, %f, %f})",
        xyzw[i] = max(arg1f[i], arg2f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_CLAMP(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];
    Result *arg2 = stack[stack_size - 2];
    Result *arg3 = stack[stack_size - 3];

    // clamp(X, 0, 1) == saturate(X)
    if (arg2->op->op == CC_EXT_0 &&
        arg3->op->op == CC_EXT_1) {

        // change op, remove 0 and 1
        op->op = CC_EXT_SATURATE;
        gfx_cc_ext_remove_result(ops, arg2);
        gfx_cc_ext_remove_result(ops, arg3);

        print_optimization("Replaced `clamp(X, 0, 1)` by `saturate(X)`.\n");
        return true;
    }

    // clamp(C1, C2, C3) == C
    check_constant_operation_3(
        "clamp({%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f})",
        xyzw[i] = clamp(arg1f[i], arg2f[i], arg3f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_SATURATE(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // saturate(C1) == C
    check_constant_operation_1(
        "saturate({%f, %f, %f, %f})",
        xyzw[i] = clamp(arg1f[i], 0, 1);
    );

    return false;
}

static bool gfx_cc_ext_optimize_COS(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // cos(C1) == C
    check_constant_operation_1(
        "cos({%f, %f, %f, %f})",
        xyzw[i] = cosf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_SIN(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // sin(C1) == C
    check_constant_operation_1(
        "sin({%f, %f, %f, %f})",
        xyzw[i] = sinf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_SQRT(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // sqrt(C1) == C
    check_constant_operation_1(
        "sqrt({%f, %f, %f, %f})",
        xyzw[i] = sqrtf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_RSQRT(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // rsqrt(C1) == C
    check_constant_operation_1(
        "rsqrt({%f, %f, %f, %f})",
        xyzw[i] = 1.f / sqrtf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_ROUND(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // round(C1) == C
    check_constant_operation_1(
        "round({%f, %f, %f, %f})",
        xyzw[i] = roundf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_FLOOR(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // floor(C1) == C
    check_constant_operation_1(
        "floor({%f, %f, %f, %f})",
        xyzw[i] = floorf(arg1f[i]);
    );

    return false;
}

static bool gfx_cc_ext_optimize_CEIL(struct GrowingArray *ops, struct GrowingArray *constants, Op *op, struct GrowingArray *results, Result **stack, size_t stack_size) {
    Result *arg1 = stack[stack_size - 1];

    // ceil(C1) == C
    check_constant_operation_1(
        "ceil({%f, %f, %f, %f})",
        xyzw[i] = ceilf(arg1f[i]);
    );

    return false;
}

static bool (*OPERATIONS[0x100])(struct GrowingArray *, struct GrowingArray *, Op *, struct GrowingArray *, Result **, size_t) = {
#define CC_EXT_OPERATOR(name, ...) [CC_EXT_##name] = gfx_cc_ext_optimize_##name,
#define CC_EXT_FUNCTION(name, ...) [CC_EXT_##name] = gfx_cc_ext_optimize_##name,
#include "gfx_cc_ext_operators.inl"
#include "gfx_cc_ext_functions.inl"
#undef CC_EXT_OPERATOR
#undef CC_EXT_FUNCTION
};

//
// Optimizations
//

static bool gfx_cc_ext_perform_optimization_pass(struct GrowingArray *ops, struct GrowingArray *constants) {
    struct GrowingArray *results = growing_array_init(NULL, 16, malloc, free);
    Result *stack[CC_EXT_STACK_SIZE] = {0};
    size_t stack_size = 0;
    bool done = false;

    for (uint32_t i = 0; i < ops->count; ++i) {
        Op *op = op_get(ops, i);

        // value
        if (op_is_value(op->op)) {

            // constants
            if (gfx_cc_ext_optimize_constant(ops, constants, op)) {
                goto cleanup;
            }

            // note: no need to check the stack size
            // validation runs before and optimization doesn't use more stack
            stack[stack_size++] = result_new(results, op, NULL, NULL, NULL, NULL);
            continue;
        }

        // operator/function
        if (OPERATIONS[op->op] && OPERATIONS[op->op](ops, constants, op, results, stack, stack_size)) {
            goto cleanup;
        }

        // pop args
        Result *args[4] = {0};
        uint8_t num_args = CC_EXT_OPERATOR_ARGS(op->op);
        for (uint8_t i = 0; i < num_args; ++i) {
            args[i] = stack[stack_size - 1 - i];
        }
        stack_size -= num_args;

        // push result
        stack[stack_size++] = result_new(results, op, args[0], args[1], args[2], args[3]);
    }

    done = true;

cleanup:
    growing_array_free(&results);
    return done;
}

void gfx_cc_ext_optimize(struct GrowingArray *ops, struct GrowingArray *constants) {
    bool done = false;
    while (!done) {
        done = gfx_cc_ext_perform_optimization_pass(ops, constants);
    }
}
