[![build](https://github.com/fmenezes/ni/actions/workflows/build.yml/badge.svg)](https://github.com/fmenezes/ni/actions/workflows/build.yml)

# ni

This is a new ahead of time compiled programing language from the ground up.

## Build

### Requirements

* Modern C++ compiler ([Clang](https://clang.llvm.org/) recommended)
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

* To generate Assembly Code
```
ni -s output.s examples/test.ni
```

* To generate Object File
```
ni -c output.o examples/test.ni
```

* To generate LLVM Intermediate Representation Code
```
ni -ll output.ll examples/test.ni
```

* To generate Binary File
```
ni -o output examples/test.ni
```

## Ni Programming Language

### Built-in types

| Types | Values |
| --- | --- |
| int, int8, int16, int32, int64, int128 | 1, 10000, -14, 0x2A
| uint, uint8, uint16, uint32, uint64, uint128 | 0, 1, 10000, 0x2A
| float16, float32, float64 | 0.34, 1234.55, -23.67 |
| bool | true, false |

Type aliases:
- byte: uint8

### Operators

| Operator | Example |
| --- | --- |
| = | a = 10 |
| + | a + 2 |
| - | a - 2 |
| * | a * 2 |
| == | a == 10 |
| != | a != 10 |
| > | a > 10 |
| < | a < 10 |
| >= | a >= 10 |
| <= | a <= 10 |
| && | a && b |
| &#124;&#124; | a &#124;&#124; b |

### Functions

```
fun square(a int) int {
    ret a * a
}
```

### Constants

```
const ten = 10
fun timesten(a int) int {
    ret a * ten
}
```

### Control flow

if

```
fun greaterThanTwo(a int) bool {
    var result bool
    if a > 2 {
        result = true
    } else {
        result = false
    }
    return result
}
```
