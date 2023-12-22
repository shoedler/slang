cls Foo {
  ctor {
    fn ctor { "bar" } // [ExpectCompileError] ERROR at [line 3] at 'ctor': Expecting variable name.
    print ctor() // [ExpectCompileError] ERROR at [line 4] at 'ctor': Expecting expression.
  }
} // [ExpectCompileError] ERROR at [line 7] at end: Expecting '}' after block.
