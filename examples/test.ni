fun run (a int, b int, c int, d int, e int, f int, g int) int {
    var k int
    k = a + b + c + d + e + f + g
    ret k
}

fun main() int {
    var a int
    a = run(10, 20, 30, 40, 50, 60, -1)
    ret a
}
