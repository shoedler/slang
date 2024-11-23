cls Foo {
  fn method0() { ret "no args" }
  fn method1(a) { ret a }
  fn method2(a, b) { ret a + b }
  fn method3(a, b, c) { ret a + b + c }
  fn method4(a, b, c, d) { ret a + b + c + d }
  fn method5(a, b, c, d, e) { ret a + b + c + d + e }
  fn method6(a, b, c, d, e, f) { ret a + b + c + d + e + f }
  fn method7(a, b, c, d, e, f, g) { ret a + b + c + d + e + f + g }
  fn method8(a, b, c, d, e, f, g, h) { ret a + b + c + d + e + f + g + h }
}

let foo = Foo()
print foo.method0() // [expect] no args
print foo.method1(1) // [expect] 1
print foo.method2(1, 2) // [expect] 3
print foo.method3(1, 2, 3) // [expect] 6
print foo.method4(1, 2, 3, 4) // [expect] 10
print foo.method5(1, 2, 3, 4, 5) // [expect] 15
print foo.method6(1, 2, 3, 4, 5, 6) // [expect] 21
print foo.method7(1, 2, 3, 4, 5, 6, 7) // [expect] 28
print foo.method8(1, 2, 3, 4, 5, 6, 7, 8) // [expect] 36
