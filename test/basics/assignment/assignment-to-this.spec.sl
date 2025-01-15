cls Foo {
  ctor {           // [exit] 2
    this = "value" // [expect-error] Parser error at line 3 at '=': Invalid assignment target.
  }                // [expect-error]      3 |     this = "value"
}                  // [expect-error]                   ~

Foo()