cls Base {
  ctor(a, b) {
    print "Base.ctor(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  ctor(a, b) {
    print "Derived.ctor(" + a + ", " + b + ")"
    base(a, b)
  }
}

Derived("a", "b")
// [Expect] Derived.ctor(a, b)
// [Expect] Base.ctor(a, b)