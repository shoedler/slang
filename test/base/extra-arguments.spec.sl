cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    print "Derived.foo()"
    base.foo("a", "b", "c", "d") // [ExpectError] Uncaught error: Expected 2 arguments but got 4.
  }                              // [ExpectError]     10 |     base.foo("a", "b", "c", "d")
}                                // [ExpectError]                   ~~~~~~~~~~~~~~~~~~~~~~
                                 // [ExpectError]   at line 10 in "foo" in module "main"
                                 // [ExpectError]   at line 14 at the toplevel of module "main"

Derived().foo() // [Expect] Derived.foo()