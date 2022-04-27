//
// Created by Simon on 06/03/2022.
//

#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "type.h"
#include "lang.h"

namespace sorth::ast {

    enum ExpressionType {
        expr_none,
        expr_operation,
        expr_operation_string,
        expr_operation_int,
        expr_scope,
        expr_if,
        expr_while
    };

    // Base class for all expressions
    struct Expression {
        virtual ExpressionType get_type() { return expr_none; };
    };

    struct OperationExpression : Expression {

        lang::Operation operation{};

        explicit OperationExpression(lang::Operation operation) : operation(operation) {}

        ExpressionType get_type() override {
            return expr_operation;
        };
    };

    template <typename ValueType>
    struct ValuedOperationExpression : OperationExpression {

        ValuedOperationExpression(lang::Operation operation, ValueType value) : OperationExpression(operation), value(value) {}

        ValueType value;

        ExpressionType get_type() override {
            if constexpr (std::is_same_v<ValueType, std::string>) {
                return expr_operation_string;
            }
            if constexpr (std::is_same_v<ValueType, int64_t>) {
                return expr_operation_int;
            }
            return expr_operation;
        };
    };

    using StringOperationExpression = ValuedOperationExpression<std::string>;
    using IntOperationExpression = ValuedOperationExpression<int64_t>;

    struct Scope : Expression {

        Scope() = default;

        Scope(Scope&) = delete;

        Scope& operator=(const Scope&) = delete;

        Scope(Scope&&) = default;

        Scope& operator=(Scope&&) = default;

        ~Scope() = default;

        type::TypeSignature signature{};
        std::vector<std::unique_ptr<Expression>> expressions;

        ExpressionType get_type() override { return expr_scope; };
    };

    struct ConditionalBranch {
        Scope condition;
        Scope body;
    };

    struct IfExpression : Expression {

        IfExpression() = default;

        IfExpression(IfExpression&) = delete;

        IfExpression(IfExpression&&) = default;

        type::TypeSignature signature;
        ConditionalBranch first_if;
        std::vector<ConditionalBranch> else_if;
        Scope else_body;

        ExpressionType get_type() override { return expr_if; };
    };

    struct WhileExpression : Expression {

        WhileExpression() = default;

        WhileExpression(WhileExpression&) = delete;

        WhileExpression(WhileExpression&&) = default;

        type::TypeSignature signature;
        Scope condition;
        Scope body;

        ExpressionType get_type() override { return expr_while; };
    };

    struct Function {
        std::string name;
        type::TypeSignature signature;
        Scope body;
    };

    struct Program {
        std::unordered_map<std::string, Function> functions;
    };
}
