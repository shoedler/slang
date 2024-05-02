cls Base {
  fn foo() {          // [Exit] 2
    base.doesNotExist // [ExpectError] Compile error at line 3 at 'base': Can't use 'base' in a class with no base class.
  }
}

Base().foo()
// [ExpectError] Compile error at line 8 at end: Expecting '}' after block.