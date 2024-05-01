cls Foo {
  ctor {
    fn ctor { "bar" } // [ExpectError] Compile error at line 3 at 'ctor': Expecting function name.
    print ctor() // [ExpectError] Compile error at line 4 at 'ctor': Expecting expression.
  }
} // [ExpectError] Compile error at line 7 at end: Expecting '}' after block.
