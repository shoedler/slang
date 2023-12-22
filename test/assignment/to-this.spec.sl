cls Foo {
  ctor {
    this = "value" // [ExpectCompileError] ERROR at [line 3] at '=': Invalid assignment target.
  }
}

Foo()
// [ExpectCompileError] ERROR at [line 8] at end: Expecting '}' after block.