cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    print "Derived.foo()"
    base.foo("a", "b", "c", "d") // [ExpectRuntimeError] Expected 2 arguments but got 4.
  }                              // [ExpectRuntimeError] at line 10 in "foo"
}                                // [ExpectRuntimeError] at line 14 at the toplevel

Derived().foo() // [Expect] Derived.foo()