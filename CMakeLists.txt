cmake_minimum_required(VERSION 3.5.0)
project(cha VERSION 0.0.1 LANGUAGES CXX C)

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
        
        # Find gcov or llvm-cov
        find_program(GCOV_PATH gcov)
        find_program(LLVM_COV_PATH llvm-cov)
        
        if(LLVM_COV_PATH)
            set(GCOV_PATH "${LLVM_COV_PATH} gcov")
        endif()
    endif()
endif()

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

add_executable(cha ${SRC} ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS})

target_link_libraries(cha ${LLVM_LIBS})

file(GLOB_RECURSE TEST_SRC
    CONFIGURE_DEPENDS
    "test/*.cha"
    "test/**/*.cha"
)

add_executable(
    cha_test
    test/test.c
)

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

# Coverage targets
if(ENABLE_COVERAGE)
    # Reset coverage counters
    add_custom_target(coverage-reset
        COMMAND find ${CMAKE_CURRENT_BINARY_DIR} -name "*.gcda" -delete
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Resetting coverage counters"
    )
    
    # Generate coverage report
    add_custom_target(coverage-report
        COMMAND mkdir -p coverage
        COMMAND gcov -r -pb ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cha.dir/src/*.gcno
        COMMAND find . -name "*.gcov" -exec mv {} coverage/ \\;
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS cha
        COMMENT "Generating coverage report"
    )
    
    # Generate HTML coverage report (requires lcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage-html
            COMMAND mkdir -p coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage/coverage.info
            COMMAND ${LCOV_PATH} --remove coverage/coverage.info '/usr/*' '*/build/*' '*/test/*' --output-file coverage/coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage/html coverage/coverage.info.cleaned
            COMMAND echo "Coverage report generated in coverage/html/index.html"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS cha
            COMMENT "Generating HTML coverage report"
        )
        
        # Combined target: run tests and generate HTML report
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} --build . --target coverage-reset
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} --build . --target coverage-html
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running tests and generating coverage report"
        )
    else()
        # Fallback: run tests and generate basic coverage report
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} --build . --target coverage-reset
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} --build . --target coverage-report
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running tests and generating coverage report"
        )
    endif()
endif()
