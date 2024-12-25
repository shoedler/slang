// [exit] 3
cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    print "Derived.foo()"
    base.foo("a", "b", "c", "d") // [expect-error] Uncaught error: Expected 2 arguments but got 4.
  }                              // [expect-error]     11 |     base.foo("a", "b", "c", "d")
}                                // [expect-error]                  ~~~~~~~~~~~~~~~~~~~~~~~~
                                 // [expect-error]   at line 11 in "foo" in module "main"
                                 // [expect-error]   at line 17 at the toplevel of module "main"

Derived().foo() // [expect] Derived.foo()