cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    base.foo(1) // [ExpectRuntimeError] Expected 2 arguments but got 1.
  }             // [ExpectRuntimeError] [line 9] in fn "foo"
}               // [ExpectRuntimeError] [line 13] in fn toplevel

Derived().foo()
