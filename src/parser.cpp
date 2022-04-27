//
// Created by Simon on 07/03/2022.
//
#include <sstream>
#include <memory>
#include <iostream>
#include "parser.h"

namespace sorth {

    ast::Scope parse_scope(Lexer& lexer, ast::Program& program, type::TypeStack& type_stack, lang::Keyword end_keyword = lang::keyword_end);

    static bool is_keyword(const Lexer::Token& token, lang::Keyword keyword) {
        return token.type == Lexer::tok_keyword && token.int_val == keyword;
    }

    template <typename... Args>
    static std::string err_message(const Args&... args) {
        std::stringstream out;
        (out << ... << args);
        out << "\n";
        return out.str();
    }

    static void recalibrate_offset(int64_t& local_offset, const type::TypeSignature& applied_signature, type::TypeSignature& output_signature) {
        local_offset -= static_cast<int64_t>(applied_signature.in.size());
        if (local_offset < 0) {
            output_signature.in.insert(output_signature.in.begin(), applied_signature.in.begin(), applied_signature.in.begin() - local_offset);
            local_offset = 0;
        }
        local_offset += static_cast<int64_t>(applied_signature.out.size());
    }

    static bool check_and_apply_signature(const type::TypeSignature& signature, type::TypeStack& type_stack) {
        auto input_count = static_cast<int64_t>(signature.in.size());
        if (!std::equal(signature.in.begin(), signature.in.end(), type_stack.end() - input_count)) return false;
        type_stack.erase(type_stack.end() - input_count, type_stack.end());
        type_stack.insert(type_stack.end(), signature.out.begin(), signature.out.end());
        return true;
    }

    static bool match_signature(const type::TypeSignature& outer, const type::TypeSignature& inner) {
        auto offset = static_cast<int64_t>(outer.in.size()) - static_cast<int64_t>(inner.in.size());
        if (outer.out.size() < offset) return false;
        for (auto i = 0; i < offset; ++i) {
            if (outer.in.at(i) != outer.out.at(i)) return false;
        }
        if (outer.out.size() - offset != inner.out.size()) return false;
        return std::equal(inner.out.begin(), inner.out.end(), outer.out.begin() + offset);
    }

    static ast::IfExpression parse_if(Lexer& lexer, ast::Program& program, type::TypeStack& type_stack) {
        assert(is_keyword(lexer.current_token(), lang::keyword_if));
        ast::IfExpression if_expr;
        auto first_condition = parse_scope(lexer, program, type_stack, lang::keyword_begin);
        if_expr.first_if.condition = std::move(first_condition);
        // todo: remove and check bool on top of the stack
        auto first_body = parse_scope(lexer, program, type_stack);
        if_expr.first_if.body = std::move(first_body);
        lexer.next_token(); // fix this
        while (is_keyword(lexer.current_token(), lang::keyword_else_if)) {
            //condition
            auto elif_condition = parse_scope(lexer, program, type_stack, lang::keyword_begin);
            auto elif_body = parse_scope(lexer, program, type_stack);
            lexer.next_token();
        }
        if (is_keyword(lexer.current_token(), lang::keyword_else)) {
            auto else_body = parse_scope(lexer, program, type_stack);
        }
        return if_expr;
    };

    static ast::WhileExpression parse_while(Lexer& lexer, ast::Program& program, type::TypeStack& type_stack) {
        throw ParseException{err_message(lexer, "While not supported yet.")};
        return {};
    };

    ast::Scope parse_scope(Lexer& lexer, ast::Program& program, type::TypeStack& type_stack, const lang::Keyword end_keyword) {
        ast::Scope scope;
        int64_t local_offset = 0;
        lexer.next_token();
        while (!is_keyword(lexer.current_token(), end_keyword)) {
            const auto token = lexer.current_token();
            switch (token.type) {
                case Lexer::tok_eof:
                    throw ParseException{err_message(lexer, "Unexpected end of file. Scope is left unclosed.")};
                case Lexer::tok_int:
                    scope.expressions.emplace_back(std::make_unique<ast::IntOperationExpression>(lang::op_push_int, token.int_val));
                    type_stack.push_back(type::int_t);
                    ++local_offset;
                    break;
                case Lexer::tok_word:
                    if (program.functions.contains(token.str_val)) {
                        const auto& signature = program.functions[token.str_val].signature;
                        if (type_stack.size() < signature.in.size())
                            throw ParseException{err_message(lexer, "Not enough data on the stack.")};
                        if (!check_and_apply_signature(signature, type_stack))
                            throw ParseException{err_message(lexer, "Required types on stack aren't matching.")};
                        recalibrate_offset(local_offset, signature, scope.signature);
                        scope.expressions.emplace_back(std::make_unique<ast::StringOperationExpression>(lang::op_call, token.str_val));
                    } else {
                        throw ParseException{err_message(lexer, "Unknown word: ", token.str_val)};
                    }
                    break;
                case Lexer::tok_keyword:
                {
                    auto keyword = static_cast<lang::Keyword>(token.int_val);
                    switch (keyword) {
                        case lang::keyword_const:
                            throw ParseException{err_message(lexer, "Const not implemented yet")};
                            break;
                        case lang::keyword_begin:
                        {
                            auto parsed_scope = parse_scope(lexer, program, type_stack);
                            recalibrate_offset(local_offset, parsed_scope.signature, scope.signature);
                            scope.expressions.emplace_back(std::make_unique<ast::Scope>(std::move(parsed_scope)));
                        }
                            break;
                        case lang::keyword_end:
                            throw ParseException{err_message(lexer, "Unexpected end of scope.")};
                            break;
                        case lang::keyword_if:
                        {
                            auto parsed_if = parse_if(lexer, program, type_stack);
                            recalibrate_offset(local_offset, parsed_if.signature, scope.signature);
                            scope.expressions.emplace_back(std::make_unique<ast::IfExpression>(std::move(parsed_if)));
                            continue; // skipping next token, cause if needs prefetching
                        }
                            break;
                        case lang::keyword_while:
                        {
                            auto parsed_while = parse_while(lexer, program, type_stack);
                            recalibrate_offset(local_offset, parsed_while.signature, scope.signature);
                            scope.expressions.emplace_back(std::make_unique<ast::WhileExpression>(std::move(parsed_while)));
                        }
                            break;
                        case lang::keyword_else:
                            throw ParseException{err_message(lexer, "Unexpected else.")};
                            break;
                        case lang::keyword_else_if:
                            throw ParseException{err_message(lexer, "Unexpected else if.")};
                            break;
                        case lang::keyword_function:
                            throw ParseException{err_message(lexer, "Functions are only allowed at toplevel.")};
                            break;
                    }
                }
                    break;
                case Lexer::tok_intrinsic:
                {
                    auto intrinsic = static_cast<lang::Intrinsic>(token.int_val);
                    if (intrinsic == lang::intrinsic_invalid)
                        throw ParseException{err_message(lexer, "Unknown Intrinsic.")};
                    if (type_stack.size() < lang::get_intrinsic_input_count(intrinsic))
                        throw ParseException{err_message(lexer, "Not enough data on the stack.")};
                    auto signature = lang::get_intrinsic_signature(intrinsic, type_stack);
                    if (!check_and_apply_signature(signature, type_stack))
                        throw ParseException{err_message(lexer, "Required types on stack aren't matching.")};
                    recalibrate_offset(local_offset, signature, scope.signature);
                    scope.expressions.emplace_back(std::make_unique<ast::OperationExpression>(lang::intrinsic_to_operation(intrinsic)));
                }
                    break;
                case Lexer::tok_str:
                    throw ParseException{err_message(lexer, "Strings are not implemented yet.")};
                    break;
                case Lexer::tok_char:
                    scope.expressions.emplace_back(std::make_unique<ast::IntOperationExpression>(lang::op_push_int, token.int_val));
                    type_stack.push_back(type::char_t);
                    ++local_offset;
                    break;
                case Lexer::tok_unexpected:
                    throw ParseException{err_message(lexer, "Unexpected token.")};
            }
            lexer.next_token();
        }
        scope.signature.out.insert(scope.signature.out.begin(), type_stack.end() - local_offset, type_stack.end());
        return scope;
    }

    static ast::Function parse_function(Lexer& lexer, ast::Program& program) {
        assert(is_keyword(lexer.current_token(), lang::keyword_function));
        // read name
        lexer.next_token();
        if (lexer.current_token().type != Lexer::tok_word) throw ParseException{err_message(lexer, "Expected word as function name")};
        auto name = lexer.current_token().str_val;
        // todo: restrict function name further
        if (program.functions.contains(name))
            throw ParseException{err_message(lexer, "Redefinition of function: ", name)};
        // read signature
        type::TypeSignature signature;
        lexer.next_token();
        // read input types
        for (; !is_keyword(lexer.current_token(), lang::keyword_begin); lexer.next_token()) {
            const auto token = lexer.current_token();
            if (token.type != Lexer::tok_word) throw ParseException{err_message(lexer, "Expected word in function signature")};
            if (token.str_val == "--") {
                lexer.next_token();
                break;
            }
            auto type = type::from_name(token.str_val);
            if (type == type::invalid_t) throw ParseException{err_message(lexer, "Unknown type ", token.str_val)};
            signature.in.push_back(type);
        }
        for (; !is_keyword(lexer.current_token(), lang::keyword_begin); lexer.next_token()) {
            const auto token = lexer.current_token();
            if (token.type != Lexer::tok_word) throw ParseException{err_message(lexer, "Expected word in function signature")};
            auto type = type::from_name(token.str_val);
            if (type == type::invalid_t) throw ParseException{err_message(lexer, "Unknown type ", token.str_val)};
            signature.out.push_back(type);
        }

        type::TypeStack type_stack{signature.in};
        auto scope = parse_scope(lexer, program, type_stack);
        if (!match_signature(signature, scope.signature))
            throw ParseException{err_message(lexer, "Function signature does not match. Expected: ", type::output_signature(signature), "but got: ", type::output_signature(scope.signature))};
        return {name, signature, std::move(scope)};
    }

    ast::Program parse_program(const std::filesystem::path& path) {
        ast::Program program;

        Lexer lexer{path};

        for (; lexer.current_token().type != Lexer::tok_eof; lexer.next_token()) {
            const auto token = lexer.current_token();
            switch (token.type) {
                case Lexer::tok_keyword:
                    if (token.int_val == lang::keyword_function) {
                        auto function = parse_function(lexer, program);
                        program.functions.insert(std::make_pair(function.name, std::move(function)));
                    } else {
                        // todo: add detail
                        throw ParseException{err_message(lexer, "Unexpected keyword: ", token.str_val)};
                    }
                    break;
                default:
                    // todo: add detail
                    throw ParseException{err_message(lexer, "Unexpected token")};
                    break;
            }
        }

        return program;
    }
}