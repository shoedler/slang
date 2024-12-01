// [exit] 3
cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    base.foo(1) // [expect-error] Uncaught error: Expected 2 arguments but got 1.
  }             // [expect-error]     10 |     base.foo(1)
}               // [expect-error]                   ~~~~~~
                // [expect-error]   at line 10 in "foo" in module "main"
                // [expect-error]   at line 16 at the toplevel of module "main"

Derived().foo()
