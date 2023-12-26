cls Foo {
  ctor {
    this = "value" // [ExpectCompileError] Compile error at line 3 at '=': Invalid assignment target.
  }
}

Foo()
// [ExpectCompileError] Compile error at line 8 at end: Expecting '}' after block.