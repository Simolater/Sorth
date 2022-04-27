//
// Created by Simon on 07/03/2022.
//
#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace sorth::type {

    using type_t = int64_t;

    using TypeStack = std::vector<type_t>;

    struct TypeSignature {
        // signature <in> -- <out> for expressions and type checking
        TypeStack in;
        TypeStack out;
    };

    constexpr int64_t basic_type_count = 3;
    enum BasicType : type_t {
        invalid_t = -1,
        int_t = 1,
        bool_t = 2,
        char_t = 3,
    };

    static type_t from_name(const std::string& name) {
        static_assert(basic_type_count == 3);
        static const std::unordered_map<std::string, BasicType> basic_types {
                {"int", int_t},
                {"bool", bool_t},
                {"char", char_t},
        };
        if (basic_types.contains(name)) {
            return basic_types.at(name);
        }
        return invalid_t;
    }

    static std::string to_name(const type_t& type) {
        static_assert(basic_type_count == 3);
        static const std::unordered_map<type_t , std::string> basic_types {
                {int_t, "int"},
                {bool_t, "bool"},
                {char_t, "char"},
        };
        if (basic_types.contains(type)) {
            return basic_types.at(type);
        }
        return "invalid";
    }

    static std::string output_stack(const TypeStack& stack) {
        std::stringstream os;
        for (const auto& type : stack) {
            os << to_name(type) << ' ';
        }
        return os.str();
    }

    static std::string output_signature(const TypeSignature& signature) {
        std::stringstream os;
        os << output_stack(signature.in) << "-- " << output_stack(signature.out);
        return os.str();
    }
}
