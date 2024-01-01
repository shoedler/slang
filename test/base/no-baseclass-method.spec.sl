cls Base {}

cls Derived : Base {
  fn foo() {
    base.doesNotExist(1) // [ExpectRuntimeError] Undefined property 'doesNotExist'.
  }                      // [ExpectRuntimeError] at line 5 in "foo"
}                        // [ExpectRuntimeError] at line 9 at the toplevel

Derived().foo()
