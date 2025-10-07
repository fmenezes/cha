[![build](https://github.com/fmenezes/cha/actions/workflows/build.yml/badge.svg)](https://github.com/fmenezes/cha/actions/workflows/build.yml)
[![coverage](https://github.com/fmenezes/cha/actions/workflows/coverage.yml/badge.svg)](https://github.com/fmenezes/cha/actions/workflows/coverage.yml)
[![Coverage Status](https://coveralls.io/repos/github/fmenezes/cha/badge.svg?branch=main)](https://coveralls.io/github/fmenezes/cha?branch=main)

# cha

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

### Compile cha programs

To compile cha programs you can simply run:

* To generate Assembly Code
```
cha -s output.s examples/test.cha
```

* To generate Object File
```
cha -c output.o examples/test.cha
```

* To generate LLVM Intermediate Representation Code
```
cha -ll output.ll examples/test.cha
```

* To generate Binary File
```
cha -o output examples/test.cha
```

### Example Programs

See the `examples/` directory for sample programs:
- `examples/test.cha` - Basic function calls and arithmetic
- `examples/unary_demo.cha` - Demonstrates unary operators and comments

## cha Programming Language

### Built-in types

| Types | Values |
| --- | --- |
| int, int8, int16, int32, int64 | 1, 10000, -14, 0x2A
| uint, uint8, uint16, uint32, uint64 | 0, 1, 10000, 0x2A
| float16, float32, float64 | 0.34, 1234.55, -23.67 |
| bool | true, false |

Type aliases:
- byte: uint8

### Operators

#### Binary Operators

| Operator | Example |
| --- | --- |
| = | a = 10 |
| + | a + 2 |
| - | a - 2 |
| * | a * 2 |
| / | a / 2 |
| == | a == 10 |
| != | a != 10 |
| > | a > 10 |
| < | a < 10 |
| >= | a >= 10 |
| <= | a <= 10 |
| && | a && b |
| &#124;&#124; | a &#124;&#124; b |

#### Unary Operators

| Operator | Example | Description |
| --- | --- | --- |
| - | -42, -a | Numeric negation |
| ! | !true, !condition | Logical NOT |

#### Operator Precedence

Operators follow standard mathematical precedence (highest to lowest):
1. Unary operators: `!`, `-` (unary)
2. Multiplicative: `*`, `/`
3. Additive: `+`, `-` (binary)
4. Comparison: `<`, `<=`, `>`, `>=`
5. Equality: `==`, `!=`
6. Logical AND: `&&`
7. Logical OR: `||`
8. Assignment: `=`

### Comments

cha supports C++-style single-line comments:

```
// This is a comment
fun example() int {
    var x int = 42  // Another comment
    ret x
}
```

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
    ret result  // Note: using 'ret' for return
}

fun absoluteValue(x int) int {
    if x < 0 {
        ret -x  // Using unary negation
    } else {
        ret x
    }
}

fun isNotZero(value int) bool {
    ret !(value == 0)  // Using unary NOT operator
}
```
