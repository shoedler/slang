cls Base {
  ctor(a, b) {
    print "Base.__ctor(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  ctor(a, b) {
    print "Derived.__ctor(" + a + ", " + b + ")"
    base(a, b)
  }
}

cls Derived2 : Derived {
  ctor(a, b) {
    print "Derived2.__ctor(" + a + ", " + b + ")"
    base.__ctor(a, b) // Also valid syntax
  }
}

Derived("a", "b")
// [Expect] Derived.__ctor(a, b)
// [Expect] Base.__ctor(a, b)

Derived2("c", "d") 
// [Expect] Derived2.__ctor(c, d)
// [Expect] Derived.__ctor(c, d)
// [Expect] Base.__ctor(c, d)
