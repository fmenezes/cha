# ni

This is a new ahead of time compiled programing language from the ground up.

## Build

### Requirements

* Modern C++ compiler ([Clang 13](https://clang.llvm.org/) recommended)
* [Bison](https://www.gnu.org/software/bison/)
* [Flex](https://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/)

### Build

To build simply run
```
make
```

### Compile ni programs

To compile ni programs you can simply run:

* To generate LLVM IR
```
ni -ll examples/test.ni output.ll
```
* To generate Assembly Code
```
ni -asm examples/test.ni output.s
```
