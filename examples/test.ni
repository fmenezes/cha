fun run {
    var a
    a = 10
    var b
    b = 20
    a = a + 30
    b = b + 40
    a = a + b
    a = a * 2
    ret a
}

fun main {
    var a
    a = run()
    ret a
}
