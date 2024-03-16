cls Foo {
  ctor {
    fn ctor { "bar" } // [ExpectCompileError] Compile error at line 3 at 'ctor': Expecting function name.
    print ctor() // [ExpectCompileError] Compile error at line 4 at 'ctor': Expecting expression.
  }
} // [ExpectCompileError] Compile error at line 7 at end: Expecting '}' after block.
