cls Base {
  fn foo() {               // [exit] 2
    base.does_not_exist(1) // [expect-error] Compile error at line 3 at 'base': Can't use 'base' in a class with no base-class.
  }
}

Base().foo()
// [expect-error] Compile error at line 8 at end: Expecting '}' after block.