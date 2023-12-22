cls Base {}

cls Derived : Base {
  fn foo() {
    base.doesNotExist(1) // [ExpectRuntimeError] Undefined property 'doesNotExist'.
  }                      // [ExpectRuntimeError] [line 5] in fn "foo"
}                        // [ExpectRuntimeError] [line 9] in fn toplevel

Derived().foo()
