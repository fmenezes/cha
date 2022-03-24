cmake_minimum_required(VERSION 3.0.0)
project(ni VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_CLANG_TIDY clang-tidy)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

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

flex_target(parser src/ast/parser.l "${GENERATED_DIR}/parser.yy.c")

bison_target(parser src/ast/parser.yy "${GENERATED_DIR}/parser.tab.cc")

include_directories(include ${GENERATED_DIR})

set_source_files_properties(${FLEX_parser_OUTPUTS} PROPERTIES LANGUAGE CXX)

add_executable(ni ${SRC} ${BISON_parser_OUTPUTS} ${FLEX_parser_OUTPUTS})

set_property(TARGET ni PROPERTY CXX_STANDARD 14)

FIND_PROGRAM(CLANG_FORMAT_EXECUTABLE clang-format DOC "path to the clang-format executable")

IF(CLANG_FORMAT_EXECUTABLE)
    EXECUTE_PROCESS(COMMAND ${CLANG_FORMAT_EXECUTABLE} --version
        OUTPUT_VARIABLE CLANG_FORMAT_version_output
        ERROR_VARIABLE CLANG_FORMAT_version_error
        RESULT_VARIABLE CLANG_FORMAT_version_result
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${CLANG_FORMAT_version_result} EQUAL 0)
        MESSAGE(SEND_ERROR "Command \"${CLANG_FORMAT_PATH} --version\" failed with output:\n${CLANG_FORMAT_version_error}")
    ELSE()
        IF("${CLANG_FORMAT_version_output}" MATCHES "version (.*)")
            SET(CLANG_FORMAT_VERSION "${CMAKE_MATCH_1}")
        ENDIF()
        MESSAGE(NOTICE "-- Found CLANG_FORMAT: ${CLANG_FORMAT_EXECUTABLE} (found version \"${CLANG_FORMAT_VERSION}\")")
    ENDIF()

    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -i
        ${SRC}
        ${HEADERS}
    )

    add_custom_target(
        check-format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -n
        --Werror
        ${SRC}
        ${HEADERS}
    )
ENDIF(CLANG_FORMAT_EXECUTABLE)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/0affd45b62371c1de44064a5a5f8817242208442.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST
        "test/*.cc"
        "test/*.cpp"
        "test/**/*.cc"
        "test/**/*.cpp"
        )

include(CTest)
enable_testing()

set(SRC_WITHOUT_MAIN ${SRC})
list(FILTER SRC_WITHOUT_MAIN EXCLUDE REGEX "main.cpp") 

add_executable(
  ni_unit_test
  ${TEST} ${SRC_WITHOUT_MAIN} ${BISON_parser_OUTPUTS} ${FLEX_parser_OUTPUTS}
)
target_link_libraries(
  ni_unit_test
  gtest_main
)

set_property(TARGET ni_unit_test PROPERTY CXX_STANDARD 14)

include(GoogleTest)
gtest_discover_tests(ni_unit_test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

