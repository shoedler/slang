cls A {
  ctor {
    this.a = 1
    this.b = 2
  }
}

print A().entries() // [expect] [[a, 1], [b, 2]]
print A()["a"]      // [expect] 1
print A()["b"]      // [expect] 2

print A() is A     // [expect] true
print A() is Obj   // [expect] true
