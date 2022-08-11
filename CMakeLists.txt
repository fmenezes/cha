cmake_minimum_required(VERSION 3.2.0)
project(ni VERSION 0.0.1 LANGUAGES CXX C)

add_subdirectory(ni)
add_subdirectory(nic)

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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
)
