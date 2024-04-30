fn f0() { ret 0 }
print f0() // [Expect] 0

fn f1(a) { ret a }
print f1(1) // [Expect] 1

fn f2(a, b) { ret a + b }
print f2(1, 2) // [Expect] 3

fn f3(a, b, c) { ret a + b + c }
print f3(1, 2, 3) // [Expect] 6

fn f4(a, b, c, d) { ret a + b + c + d }
print f4(1, 2, 3, 4) // [Expect] 10

fn f5(a, b, c, d, e) { ret a + b + c + d + e }
print f5(1, 2, 3, 4, 5) // [Expect] 15

fn f6(a, b, c, d, e, f) { ret a + b + c + d + e + f }
print f6(1, 2, 3, 4, 5, 6) // [Expect] 21

fn f7(a, b, c, d, e, f, g) { ret a + b + c + d + e + f + g }
print f7(1, 2, 3, 4, 5, 6, 7) // [Expect] 28

fn f8(a, b, c, d, e, f, g, h) -> a + b + c + d + e + f + g + h // Also nice as a lambda
print f8(1, 2, 3, 4, 5, 6, 7, 8) // [Expect] 36
