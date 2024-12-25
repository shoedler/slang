cls Foo {
  ctor {              // [exit] 2
    fn ctor { "bar" } // [expect-error] Parser error at line 3 at 'ctor': Expecting function name.
    print ctor()      // [expect-error]      3 |     fn ctor { "bar" }
  }                   // [expect-error]                 ~~~~
}
