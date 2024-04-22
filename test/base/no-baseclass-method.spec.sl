cls Base {}

cls Derived : Base {
  fn foo() {
    base.doesNotExist(1) // [ExpectRuntimeError] Uncaught error: Undefined method 'doesNotExist' in type Base or any of its parent classes.
  }                      // [ExpectRuntimeError] at line 5 in "foo" in module "main"
}                        // [ExpectRuntimeError] at line 9 at the toplevel of module "main"

Derived().foo()
