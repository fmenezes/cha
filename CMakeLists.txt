cmake_minimum_required(VERSION 3.5.0)
project(cha VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DCMAKE_PROJECT_NAME="${CMAKE_PROJECT_NAME}" -DCMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

include(CTest)
enable_testing()

# Add coverage option
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

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

    # windowsmanifest
    # xray
    # libdriver
    # dlltooldriver
    # coverage
    # lineeditor
    # xcoredisassembler
    # xcorecodegen
    # xcoredesc
    # xcoreinfo
    # x86targetmca
    # x86disassembler
    # x86asmparser
    x86codegen

    # x86desc
    x86info

    # webassemblydisassembler
    # webassemblyasmparser
    # webassemblycodegen
    # webassemblydesc
    # webassemblyutils
    # webassemblyinfo
    # vedisassembler
    # veasmparser
    # vecodegen
    # vedesc
    # veinfo
    # systemzdisassembler
    # systemzasmparser
    # systemzcodegen
    # systemzdesc
    # systemzinfo
    # sparcdisassembler
    # sparcasmparser
    # sparccodegen
    # sparcdesc
    # sparcinfo
    # riscvdisassembler
    # riscvasmparser
    # riscvcodegen
    # riscvdesc
    # riscvinfo
    # powerpcdisassembler
    # powerpcasmparser
    # powerpccodegen
    # powerpcdesc
    # powerpcinfo
    # nvptxcodegen
    # nvptxdesc
    # nvptxinfo
    # msp430disassembler
    # msp430asmparser
    # msp430codegen
    # msp430desc
    # msp430info
    # mipsdisassembler
    # mipsasmparser
    # mipscodegen
    # mipsdesc
    # mipsinfo
    # lanaidisassembler
    # lanaicodegen
    # lanaiasmparser
    # lanaidesc
    # lanaiinfo
    # hexagondisassembler
    # hexagoncodegen
    # hexagonasmparser
    # hexagondesc
    # hexagoninfo
    # bpfdisassembler
    # bpfasmparser
    # bpfcodegen
    # bpfdesc
    # bpfinfo
    # avrdisassembler
    # avrasmparser
    # avrcodegen
    # avrdesc
    # avrinfo
    # armdisassembler
    # armasmparser
    armcodegen

    # armdesc
    # armutils
    arminfo

    # amdgputargetmca
    # amdgpudisassembler
    # amdgpuasmparser
    # amdgpucodegen
    # amdgpudesc
    # amdgpuutils
    # amdgpuinfo
    # aarch64disassembler
    # aarch64asmparser
    aarch64codegen

    # aarch64desc
    # aarch64utils
    aarch64info

    # orcjit
    # mcjit
    # jitlink
    # interpreter
    # executionengine
    # runtimedyld
    # orctargetprocess
    # orcshared
    # dwp
    # symbolize
    # debuginfopdb
    # debuginfogsym
    # option
    # objectyaml
    # mca
    # mcdisassembler
    # lto
    # cfguard
    # frontendopenacc
    # extensions
    # Polly
    # PollyISL
    # passes
    # objcarcopts
    # coroutines
    # ipo
    # instrumentation
    # vectorize
    # linker
    # frontendopenmp
    # dwarflinker
    # globalisel
    # mirparser
    asmprinter

    # debuginfomsf
    # selectiondag
    codegen

    # irreader
    # asmparser
    # interfacestub
    # filecheck
    # fuzzmutate
    # target
    # scalaropts
    # instcombine
    # aggressiveinstcombine
    # transformutils
    # bitwriter
    # analysis
    # profiledata
    # debuginfodwarf
    # object
    # textapi
    # mcparser
    mc

    # debuginfocodeview
    # bitreader
    core

    # remarks
    # bitstreamreader
    # binaryformat
    # tablegen
    support

    # demangle
)

add_executable(cha ${CHA_SOURCES})

target_link_libraries(cha ${LLVM_LIBS})

file(GLOB_RECURSE TEST_SRC
    CONFIGURE_DEPENDS
    "test/integration/*.cha"
    "test/integration/**/*.cha"
)

add_executable(
    cha_test
    test/integration/test.c
)

# Add C++ AST unit tests
add_executable(
    ast_cpp_test
    test/unit/test_ast_cpp.cpp
    src/ast.cpp
)

# Set C++ standard for the test
set_target_properties(ast_cpp_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add the C++ AST test to CTest
add_test(NAME ast_cpp_unit_tests COMMAND ast_cpp_test)

# Add C++ Parser unit tests
add_executable(
    parser_cpp_test
    test/unit/test_parser_cpp.cpp
    src/ast.cpp
    src/log.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
)

# Set C++ standard and link dependencies for parser test
set_target_properties(parser_cpp_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(parser_cpp_test ${LLVM_LIBS})

# Add the C++ Parser test to CTest
add_test(NAME parser_cpp_unit_tests COMMAND parser_cpp_test)
set_tests_properties(parser_cpp_unit_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add C++ Validation unit tests
add_executable(
    validate_cpp_test
    test/unit/test_validate_cpp.cpp
    src/ast.cpp
    src/validate.cpp
    src/log.cpp
)

# Set C++ standard for the validation test
set_target_properties(validate_cpp_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add the C++ Validation test to CTest
add_test(NAME validate_cpp_unit_tests COMMAND validate_cpp_test)

FOREACH(test ${TEST_SRC})
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    GET_FILENAME_COMPONENT(TPath ${test} DIRECTORY)
    ADD_TEST(NAME ${TName} COMMAND cha_test "${TPath}/${TName}.cha")
    SET_PROPERTY(TEST ${TName} PROPERTY ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PATH}")
ENDFOREACH(test)

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
