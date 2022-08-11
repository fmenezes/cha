cmake_minimum_required(VERSION 3.2.0)
project(nic VERSION 0.0.1 LANGUAGES C CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC
    CONFIGURE_DEPENDS
    "src/*.c"
    "src/*.h"
    "src/**/*.c"
    "src/**/*.h"
    "include/*.h"
    "include/**/*.h"
)

flex_target(scanner src/scanner.l "${CMAKE_CURRENT_BINARY_DIR}/scanner.c")
bison_target(parser src/parser.y "${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c")
add_flex_bison_dependency(scanner parser)

include_directories(include ${CMAKE_CURRENT_BINARY_DIR})

add_executable(nic ${SRC} ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS})
