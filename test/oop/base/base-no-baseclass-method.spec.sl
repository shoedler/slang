cls Base {}

cls Derived : Base {
  fn foo() {
    base.doesNotExist(1) // [ExpectError] Uncaught error: Undefined callable 'doesNotExist' in type Base or any of its parent classes.
  }                      // [ExpectError]      5 |     base.doesNotExist(1)
}                        // [ExpectError]                   ~~~~~~~~~~~~~~~
                         // [ExpectError]   at line 5 in "foo" in module "main"
                         // [ExpectError]   at line 11 at the toplevel of module "main"

Derived().foo()
