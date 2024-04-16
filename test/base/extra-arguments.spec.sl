cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    print "Derived.foo()"
    base.foo("a", "b", "c", "d") // [ExpectRuntimeError] Uncaught error: Expected 2 arguments but got 4.
  }                              // [ExpectRuntimeError]   at line 10 in native "to_str"
}                                // [ExpectRuntimeError]   at line 10 in "foo" in module "main"
                                 // [ExpectRuntimeError]   at line 15 at the toplevel of module "main"

Derived().foo() // [Expect] Derived.foo()