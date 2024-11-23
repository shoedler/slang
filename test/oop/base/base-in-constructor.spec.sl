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
// [expect] Derived.ctor(a, b)
// [expect] Base.ctor(a, b)