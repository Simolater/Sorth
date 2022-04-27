cmake_minimum_required(VERSION 3.21)
project(sorth)

set(CMAKE_CXX_STANDARD 23)

add_executable(sorth main.cpp src/lexer.h src/lang.h src/ast.h src/type.h src/parser.h src/parser.cpp)
