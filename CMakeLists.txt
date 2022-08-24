cmake_minimum_required(VERSION 3.2.0)
project(ni VERSION 0.0.1 LANGUAGES CXX C)

add_subdirectory(ni)
add_subdirectory(nic)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
)
