//
// Created by Simon on 07/03/2022.
//

#pragma once

#include <filesystem>
#include <exception>
#include "ast.h"
#include "lexer.h"
#include "type.h"

namespace sorth {

    ast::Program parse_program(const std::filesystem::path& path);

    struct ParseException : public std::runtime_error {
        explicit ParseException(const std::string& message) : std::runtime_error(message) {}
    };
}