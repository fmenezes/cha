[![build](https://github.com/fmenezes/ni/actions/workflows/build.yml/badge.svg)](https://github.com/fmenezes/ni/actions/workflows/build.yml)

# ni

This is a new ahead of time compiled programing language from the ground up.

## Build

### Requirements

* Modern C++ compiler ([Clang 13](https://clang.llvm.org/) recommended)
* [CMake](https://cmake.org/)
* [Bison](https://www.gnu.org/software/bison/)
* [Flex](https://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/)
* [LLVM](https://https://llvm.org/)

### Build

To build simply run
```
mkdir build
cd build
cmake ..
cmake --build .
```

### Compile ni programs

To compile ni programs you can simply run:

* To generate Intermediate Representation Code
```
ni -ir examples/test.ni output.ir
```

* To generate Assembly Code
```
ni -asm examples/test.ni output.s
```

* To generate LLVM Intermediate Representation Code
```
ni -ll examples/test.ni output.ll
```
