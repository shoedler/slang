cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    base.foo(1) // [expect-error] Uncaught error: Expected 2 arguments but got 1.
  }             // [expect-error]      9 |     base.foo(1)
}               // [expect-error]                   ~~~~~~
                // [expect-error]   at line 9 in "foo" in module "main"
                // [expect-error]   at line 15 at the toplevel of module "main"

Derived().foo()
