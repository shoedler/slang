cls Base {
  ctor(a, b) {
    print "Base.__ctor(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  ctor() {
    print "Derived.__ctor()"
    base.__ctor("a", "b")
  }
}

Derived()
// [Expect] Derived.__ctor()
// [Expect] Base.__ctor(a, b)
