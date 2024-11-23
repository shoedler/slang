cls Foo {
  ctor {              // [exit] 2
    fn ctor { "bar" } // [expect-error] Compile error at line 3 at 'ctor': Expecting function name.
    print ctor() // [expect-error] Compile error at line 4 at 'ctor': Expecting expression.
  }
} // [expect-error] Compile error at line 7 at end: Expecting '}' after block.
