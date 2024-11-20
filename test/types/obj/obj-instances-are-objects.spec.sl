cls A {
  ctor {
    this.a = 1
    this.b = 2
  }
}

print A().entries() // [Expect] [[a, 1], [b, 2]]
print A()["a"]      // [Expect] 1
print A()["b"]      // [Expect] 2

print A() is A     // [Expect] true
print A() is Obj   // [Expect] true
