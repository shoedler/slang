cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    base.foo(1) // [ExpectError] Uncaught error: Expected 2 arguments but got 1.
  }             // [ExpectError]      9 |     base.foo(1)
}               // [ExpectError]                   ~~~~~
                // [ExpectError]   at line 9 in "foo" in module "main"
                // [ExpectError]   at line 13 at the toplevel of module "main"

Derived().foo()
