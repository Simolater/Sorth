//
// Created by Simon on 02/03/2022.
//
#pragma once

#include <filesystem>
#include <fstream>
#include <utility>
#include <optional>
#include <cassert>
#include <tuple>
#include <unordered_map>

#include "lang.h"

namespace sorth {

    class Lexer {

    public:

        static constexpr int64_t token_type_count = 7;
        enum TokenType {
            tok_eof = -1,
            tok_int,
            tok_word,
            tok_keyword,
            tok_intrinsic,
            tok_str,
            tok_char,
            tok_unexpected
        };

        static std::string token_type_to_str(TokenType token_type) {
            static_assert(token_type_count == 7);
            switch (token_type) {
                case tok_eof:
                    return "eof";
                case tok_int:
                    return "int";
                case tok_word:
                    return "word";
                case tok_keyword:
                    return "keyword";
                case tok_intrinsic:
                    return "intrinsic";
                case tok_str:
                    return "string";
                case tok_char:
                    return "char";
                case tok_unexpected:
                    return "unexpected";
                default:
                    return "unknown";
            }
        }

        struct Location {
            int64_t line;
            int64_t column;
        };

        struct Token {
            TokenType type{tok_unexpected};
            std::string str_val{};
            int64_t int_val{0};
            Location location{1, 0};
        };

        explicit Lexer(std::filesystem::path path) : m_path(std::move(path)), m_location({1, 0}), m_ifstream(m_path) {
            next_token();
        }

        const Token& next_token() {
            m_current_token = interpret_next_token();
            return m_current_token;
        }

        const Token& current_token() {
            return m_current_token;
        }

        friend std::ostream& operator<<(std::ostream& os, const Lexer& lexer) {
            os << lexer.m_path.native() << ':' << lexer.m_current_token.location.line << ':' << lexer.m_current_token.location.column << ": ";
            return os;
        }

    private:
        std::filesystem::path m_path;
        Location m_location;
        std::ifstream m_ifstream;
        Token m_current_token;

        Token interpret_next_token() {
            auto [has_value, word, location] = get_next_word();
            if (!has_value) return {tok_eof, "", 0, location};
            auto first = word.at(0);
            static_assert(token_type_count == 7);
            switch (first) {
                case '\'':
                    if (word.size() == 1 || word.at(word.size() - 1) != '\'') return {tok_unexpected, "Open \' has to be closed", 0, location};
                    word = word.substr(1, word.size() - 2);
                    if (auto c = parse_char(word); c.has_value()) {
                        return {tok_char, word, static_cast<unsigned char>(c.value()), location};
                    }
                    return {tok_unexpected, "Failed to parse char", 0, location};
                case '\"':
                {
                    if (!word.ends_with('\"')) return {tok_unexpected, "Unenclosed string", 0, location};
                    return {tok_str, word.substr(1, word.size() - 2), 0, location};
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    try {
                        return {tok_int, word, std::stol(word), location};
                    } catch (std::invalid_argument const& ex) {
                        return {tok_unexpected, "Invalid number", 0, location};
                    } catch (std::out_of_range const& ex) {
                        return {tok_unexpected, "Invalid number", 0, location};
                    }
                default:
                    static_assert(lang::keyword_count == 8);
                    static const std::unordered_map<std::string, lang::Keyword> keywords {
                            {"func", lang::keyword_function},
                            {"const", lang::keyword_const},
                            {"{", lang::keyword_begin},
                            {"}", lang::keyword_end},
                            {"if", lang::keyword_if},
                            {"else", lang::keyword_else},
                            {"elif", lang::keyword_else_if},
                            {"while", lang::keyword_while},
                    };
                    static_assert(lang::intrinsic_count == 15);
                    static const std::unordered_map<std::string, lang::Intrinsic> intrinsics {
                            {"+", lang::intrinsic_add},
                            {"-", lang::intrinsic_sub},
                            {"*", lang::intrinsic_mul},
                            {"/", lang::intrinsic_div},
                            {"drop", lang::intrinsic_drop},
                            {"swap", lang::intrinsic_swap},
                            {"dup", lang::intrinsic_dup},

                            {"and", lang::intrinsic_and},
                            {"or", lang::intrinsic_or},
                            {"xor", lang::intrinsic_xor},
                            {"not", lang::intrinsic_not},

                            {"=", lang::intrinsic_equal},
                            {"<", lang::intrinsic_less},
                            {">", lang::intrinsic_greater},
                    };
                    if (keywords.contains(word)) {
                        return {tok_keyword, word, keywords.at(word), location};
                    }
                    if (intrinsics.contains(word)) {
                        return {tok_intrinsic, word, intrinsics.at(word), location};
                    }
                    return {tok_word, std::move(word), 0, location};
            }
        }

        std::tuple<bool, std::string, Location> get_next_word() {
            std::stringstream word;
            char var = 0;
            do {
                if (!m_ifstream.get(var)) return {false, "", m_location};
                ++m_location.column;
                if (var == '\n') {
                    m_location.column = 0;
                    ++m_location.line;
                }
            } while (is_white_space(var));
            Location loc = m_location;
            bool is_str = var == '\"';
            do {
                word << var;
                if (!m_ifstream.get(var)) return {true, word.str(), loc};
                ++m_location.column;
                if (var == '\n') {
                    m_location.column = 0;
                    ++m_location.line;
                    return {true, word.str(), loc};
                }
            } while (is_str ? var != '\"' : !is_white_space(var));
            if (is_str) word << '\"';
            return {true, word.str(), loc};
        }

        static std::optional<char> parse_char(const std::string& word) {
            if (word.empty()) return std::nullopt;
            if (word.size() == 1) return word.at(0);
            return std::nullopt;
        }

        static bool is_white_space(char c) {
            switch (c) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    return true;
                default:
                    return false;
            }
        }
    };
}
