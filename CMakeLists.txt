cmake_minimum_required(VERSION 3.5.0)
project(cha VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DCMAKE_PROJECT_NAME="${CMAKE_PROJECT_NAME}" -DCMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

include(CTest)
enable_testing()

# Add coverage option
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include GoogleTest module for test discovery
include(GoogleTest)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM 21.1.1 REQUIRED CONFIG)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Coverage configuration
if(ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        
        find_program(LLVM_COV_PATH llvm-cov)
        find_program(LCOV_PATH lcov)
    endif()
endif()

# Setup FLEX and BISON first
flex_target(scanner src/scanner.l "${CMAKE_CURRENT_BINARY_DIR}/scanner.c")
bison_target(parser src/parser.y "${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/parser.tab.hpp")
add_flex_bison_dependency(scanner parser)

file(GLOB_RECURSE SRC
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
    "src/**/*.cpp"
    "src/**/*.hpp"
    "include/*.h"
    "include/*.hpp"
    "include/**/*.h"
    "include/**/*.hpp"
)

# Add specific C++ files we want to include (now that BISON/FLEX outputs are defined)
set(CHA_SOURCES
    src/ast.cpp
    src/validate.cpp
    src/log.cpp
    src/codegen.cpp
    src/cha.cpp
    src/main.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
)

include_directories(include src ${CMAKE_CURRENT_BINARY_DIR} ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(LLVM_LIBS
    # Core components
    core
    support
    mc
    codegen
    asmprinter

    # X86 target (for Linux builds)
    x86codegen
    x86info
    x86asmparser

    # ARM target (for ARM-based systems)
    armcodegen
    arminfo
    armasmparser

    # AArch64 target (for ARM64/Apple Silicon)
    aarch64codegen
    aarch64info
    aarch64asmparser

    # Additional components needed for code generation
    target
    analysis
    transformutils
    scalaropts
    instcombine
    vectorize
    bitwriter
    mcparser
)

add_executable(cha ${CHA_SOURCES})

# Add compiler flags to handle LLVM ABI conflicts
target_compile_options(cha PRIVATE
    -Wno-deprecated-declarations
    -D_GLIBCXX_USE_CXX11_ABI=1
)

target_link_libraries(cha ${LLVM_LIBS})

# Create a single test executable with all unit and integration tests
add_executable(
    all_tests
    test/unit/test_ast.cpp
    test/unit/test_parser.cpp
    test/unit/test_validate.cpp
    test/integration/test_integration.cpp
    src/ast.cpp
    src/validate.cpp
    src/log.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
)

# Set C++ standard and include directories for tests
set_target_properties(all_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add compiler flags to handle LLVM/GTest ABI conflicts
target_compile_options(all_tests PRIVATE
    -Wno-deprecated-declarations
    -D_GLIBCXX_USE_CXX11_ABI=1
)

target_include_directories(all_tests PRIVATE 
    include 
    src 
    ${CMAKE_CURRENT_BINARY_DIR} 
    ${LLVM_INCLUDE_DIRS}
)

# Link against Google Test and LLVM libraries
target_link_libraries(all_tests 
    gtest_main
    gtest
    ${LLVM_LIBS}
)

# Discover and register individual Google Tests with CTest
gtest_discover_tests(all_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Add a convenience target to run tests
add_custom_target(test_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all_tests
    COMMAND ${CMAKE_SOURCE_DIR}/build/all_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS all_tests
    COMMENT "Building and running all tests"
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
)

if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND mkdir -p coverage
        COMMAND lcov -c --directory CMakeFiles/cha.dir/src --output-file lcov.info
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS cha
        COMMENT "Generating coverage report"
    )
endif()

# Find clang-format
find_program(CLANG_FORMAT_EXECUTABLE
    NAMES clang-format clang-format-18 clang-format-17 clang-format-16 clang-format-15
    DOC "clang-format executable"
)

# Add format target if clang-format is found
if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.h"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/**/*.h"
        "${CMAKE_SOURCE_DIR}/include/**/*.hpp"
        "${CMAKE_SOURCE_DIR}/test/*.h"
        "${CMAKE_SOURCE_DIR}/test/*.hpp"
        "${CMAKE_SOURCE_DIR}/test/*.c"
        "${CMAKE_SOURCE_DIR}/test/*.cpp"
        "${CMAKE_SOURCE_DIR}/test/**/*.h"
        "${CMAKE_SOURCE_DIR}/test/**/*.hpp"
        "${CMAKE_SOURCE_DIR}/test/**/*.c"
        "${CMAKE_SOURCE_DIR}/test/**/*.cpp"
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format on source files"
        VERBATIM
    )
    
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking code formatting with clang-format"
        VERBATIM
    )
    
    message(STATUS "clang-format found: ${CLANG_FORMAT_EXECUTABLE}")
else()
    message(STATUS "clang-format not found. Format targets will not be available.")
endif()
