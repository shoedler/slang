cls Base {
  ctor(a, b) {
    print "Base.init(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  ctor() {
    print "Derived.init()"
    base.ctor("a", "b")
  }
}

Derived()
// [Expect] Derived.init()
// [Expect] Base.init(a, b)
