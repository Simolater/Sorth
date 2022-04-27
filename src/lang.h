//
// Created by Simon on 06/03/2022.
//
#pragma once

#include <cstdint>
#include <cassert>

#include "type.h"

namespace sorth::lang {

    static constexpr int64_t keyword_count = 8;
    enum Keyword {
        keyword_function,
        keyword_const,
        keyword_begin,
        keyword_end,
        keyword_if,
        keyword_else,
        keyword_else_if,
        keyword_while,
    };

    static constexpr int64_t intrinsic_count = 15;
    enum Intrinsic : int64_t {
        intrinsic_invalid,
        // arithmetic
        intrinsic_add,
        intrinsic_sub,
        intrinsic_mul,
        intrinsic_div,
        // logic
        intrinsic_and,
        intrinsic_or,
        intrinsic_xor,
        intrinsic_not,
        // stack ops
        intrinsic_drop,
        intrinsic_swap,
        intrinsic_dup,
        // comparisons
        intrinsic_equal,
        intrinsic_less,
        intrinsic_greater,
    };

    static constexpr int64_t operation_count = 17;
    enum Operation : int64_t {
        op_none,
        op_push_int,
        op_call,
        // arithmetic
        op_add,
        op_sub,
        op_mul,
        op_div,
        // logic
        op_and,
        op_or,
        op_xor,
        op_not,
        // stack
        op_drop,
        op_dup,
        op_swap,
        // comparisons
        op_equal,
        op_less,
        op_greater,
    };

    static int64_t get_intrinsic_input_count(Intrinsic intrinsic) {
        static_assert(intrinsic_count == 15);
        switch (intrinsic) {
            case intrinsic_drop:
            case intrinsic_dup:
                return 1;
            case intrinsic_add:
            case intrinsic_sub:
            case intrinsic_mul:
            case intrinsic_div:
            case intrinsic_swap:
            case intrinsic_and:
            case intrinsic_or:
            case intrinsic_xor:
            case intrinsic_not:
            case intrinsic_equal:
            case intrinsic_less:
            case intrinsic_greater:
                return 2;
            default:
                return -1;
        }
    }

    static Operation intrinsic_to_operation(Intrinsic intrinsic) {
        static_assert(intrinsic_count == 15);
        switch (intrinsic) {
            case intrinsic_invalid:
                return op_none;
            case intrinsic_add:
                return op_add;
            case intrinsic_sub:
                return op_sub;
            case intrinsic_mul:
                return op_mul;
            case intrinsic_div:
                return op_div;
            case intrinsic_and:
                return op_and;
            case intrinsic_or:
                return op_or;
            case intrinsic_xor:
                return op_xor;
            case intrinsic_not:
                return op_not;
            case intrinsic_drop:
                return op_drop;
            case intrinsic_swap:
                return op_swap;
            case intrinsic_dup:
                return op_dup;
            case intrinsic_equal:
                return op_equal;
            case intrinsic_less:
                return op_less;
            case intrinsic_greater:
                return op_greater;
        }
        return op_none;
    }

    static type::TypeSignature get_intrinsic_signature(Intrinsic intrinsic, const type::TypeStack& type_stack) {
        static_assert(intrinsic_count == 15);
        assert(type_stack.size() >= get_intrinsic_input_count(intrinsic));
        switch (intrinsic) {
            case intrinsic_add:
            case intrinsic_sub:
            case intrinsic_mul:
            case intrinsic_div:
            case intrinsic_and:
            case intrinsic_or:
            case intrinsic_xor:
            case intrinsic_not:
                // todo: dynamic typing for add to support i.e. floats
                return {{type::int_t, type::int_t}, {type::int_t}};
                break;
            case intrinsic_drop:
            {
                auto first = type_stack.back();
                return {{first}, {}};
            }
            case intrinsic_swap:
            {
                auto first = type_stack.back();
                auto second = type_stack[type_stack.size() - 1];
                return {{second, first}, {first, second}};
            }
            case intrinsic_dup:
            {
                auto first = type_stack.back();
                return {{first}, {first, first}};
            }
            case intrinsic_equal:
            case intrinsic_less:
            case intrinsic_greater:
                // todo: support different types
                return {{type::int_t, type::int_t}, {type::bool_t}};
            default:
                return {};
        }
    };
}
