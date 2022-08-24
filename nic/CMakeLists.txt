cmake_minimum_required(VERSION 3.2.0)
project(nic VERSION 0.0.1 LANGUAGES C CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM 14 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

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

include_directories(include src/include ${CMAKE_CURRENT_BINARY_DIR} ${LLVM_INCLUDE_DIRS})
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

add_executable(nic ${SRC} ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS})

target_link_libraries(nic ${LLVM_LIBS})
