cls Base {
  fn foo() {
    base.doesNotExist(1) // [ExpectCompileError] Compile error at line 3 at 'base': Can't use 'base' in a class with no base class.
  }
}

Base().foo()
// [ExpectCompileError] Compile error at line 8 at end: Expecting '}' after block.