cls Base {}

cls Derived : Base {
  fn foo() {
    base.does_not_exist(1) // [expect-error] Uncaught error: Undefined callable 'does_not_exist' in type Base or any of its parent classes.
  }                      // [expect-error]      5 |     base.does_not_exist(1)
}                        // [expect-error]                   ~~~~~~~~~~~~~~~~~
                         // [expect-error]   at line 5 in "foo" in module "main"
                         // [expect-error]   at line 11 at the toplevel of module "main"

Derived().foo()
