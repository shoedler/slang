cls Base {
  fn foo() {
    base.doesNotExist(1) // [ExpectCompileError] ERROR at [line 3] at 'base': Can't use 'base' in a class with no base class.
  }
}

Base().foo()
// [ExpectCompileError] ERROR at [line 8] at end: Expecting '}' after block.