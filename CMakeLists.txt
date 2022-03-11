cmake_minimum_required(VERSION 3.0.0)
project(ni VERSION 0.0.1 LANGUAGES CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

find_program (
          CLANG_FORMAT_PATH
          clang-format )

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

file(MAKE_DIRECTORY ${GENERATED_DIR})

file(GLOB SRC
        "src/*.cc"
        "src/*.cpp"
        "src/**/*.cc"
        "src/**/*.cpp"
        )

file(GLOB HEADERS
        "include/*.h"
        "include/*.hh"
        "include/**/*.h"
        "include/**/*.hh"
        )

add_custom_target(
        check-format
        COMMAND ${CLANG_FORMAT_PATH}
        -n
        --Werror
        ${SRC}
        ${HEADERS}
)

add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_PATH}
        -i
        ${SRC}
        ${HEADERS}
)

flex_target(parser src/ast/parser.l "${GENERATED_DIR}/parser.yy.c")

bison_target(parser src/ast/parser.yy "${GENERATED_DIR}/parser.tab.cc")

include_directories(include ${GENERATED_DIR})

set_source_files_properties(${FLEX_parser_OUTPUTS} PROPERTIES LANGUAGE CXX)

add_executable(ni ${SRC} ${BISON_parser_OUTPUTS} ${FLEX_parser_OUTPUTS})

set_property(TARGET ni PROPERTY CXX_STANDARD 14)
