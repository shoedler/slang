cls Base {
  fn foo(a, b) {
    print "Base.foo(" + a + ", " + b + ")"
  }
}

cls Derived : Base {
  fn foo() {
    base.foo(1) // [ExpectRuntimeError] Expected 2 arguments but got 1.
  }             // [ExpectRuntimeError] at line 9 in "foo" in module "main"
}               // [ExpectRuntimeError] at line 13 at the toplevel of module "main"

Derived().foo()
