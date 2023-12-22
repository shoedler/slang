cls Foo {
  ctor {
    ret "result"; // [ExpectCompileError] ERROR at [line 3] at 'ret': Can't return a value from a constructor.
  }
}
// [ExpectCompileError] ERROR at [line 6] at end: Expecting '}' after block.