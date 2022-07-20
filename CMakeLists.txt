cmake_minimum_required(VERSION 3.0.2)
project(ni VERSION 0.0.1 LANGUAGES CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

file(MAKE_DIRECTORY ${GENERATED_DIR})

file(GLOB_RECURSE SRC
    CONFIGURE_DEPENDS
    "src/*.c"
    "src/*.cc"
    "src/*.cpp"
    "src/**/*.c"
    "src/**/*.cc"
    "src/**/*.cpp"
)

file(GLOB_RECURSE HEADERS
    CONFIGURE_DEPENDS
    "include/*.h"
    "include/*.hh"
    "include/**/*.h"
    "include/**/*.hh"
)

flex_target(parser src/parse/parser.l "${GENERATED_DIR}/parser.yy.c")

bison_target(parser src/parse/parser.yy "${GENERATED_DIR}/parser.tab.cc")

include_directories(include src/include ${CMAKE_CURRENT_BINARY_DIR})

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
        MESSAGE(SEND_ERROR "Command \"${CLANG_FORMAT_EXECUTABLE} --version\" failed with output:\n${CLANG_FORMAT_version_error}")
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
        ${TEST_SRC}
    )

    add_custom_target(
        check-format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -n
        --Werror
        ${SRC}
        ${HEADERS}
        ${TEST_SRC}
    )
ENDIF(CLANG_FORMAT_EXECUTABLE)

file(GLOB_RECURSE TEST_LIB_SRC
    CONFIGURE_DEPENDS
    "test/lib/*.c"
    "test/lib/*.cc"
    "test/lib/*.cpp"
    "test/lib/**/*.c"
    "test/lib/**/*.cc"
    "test/lib/**/*.cpp"
)
string(REPLACE "${CMAKE_SOURCE_DIR}/" ";" TEST_LIB_SRC ${TEST_LIB_SRC})

file(GLOB_RECURSE TEST_SRC
    CONFIGURE_DEPENDS
    "test/*.c"
    "test/*.cc"
    "test/*.cpp"
    "test/**/*.c"
    "test/**/*.cc"
    "test/**/*.cpp"
)
string(REPLACE "${CMAKE_SOURCE_DIR}/" ";" TEST_SRC ${TEST_SRC})
list(REMOVE_ITEM TEST_SRC ${TEST_LIB_SRC})

set(SRC_WITHOUT_MAIN ${SRC} ${BISON_parser_OUTPUTS} ${FLEX_parser_OUTPUTS})
list(FILTER SRC_WITHOUT_MAIN EXCLUDE REGEX "main.cpp")

include(CTest)
enable_testing()

CREATE_TEST_SOURCELIST(ni_tests
    ni_unit_test_main.cpp
    ${TEST_SRC}
)

file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/ni_unit_test_main.cpp" "${GENERATED_DIR}/ni_unit_test_main.cpp")

add_executable(
    ni_unit_test
    "${GENERATED_DIR}/ni_unit_test_main.cpp" ${TEST_SRC} ${TEST_LIB_SRC} ${SRC_WITHOUT_MAIN}
)

target_include_directories(ni_unit_test PUBLIC "test/include")

set_property(TARGET ni_unit_test PROPERTY CXX_STANDARD 14)

FOREACH(test ${TEST_SRC})
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    GET_FILENAME_COMPONENT(TPath ${test} DIRECTORY)
    ADD_TEST(NAME ${TName} COMMAND ni_unit_test "${TPath}/${TName}" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
ENDFOREACH(test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
)
