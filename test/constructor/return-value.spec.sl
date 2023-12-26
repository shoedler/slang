cls Foo {
  ctor {
    ret "result"; // [ExpectCompileError] Compile error at line 3 at 'ret': Can't return a value from a constructor.
  }
}
// [ExpectCompileError] Compile error at line 6 at end: Expecting '}' after block.