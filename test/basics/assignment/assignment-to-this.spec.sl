cls Foo {
  ctor {           // [exit] 2
    this = "value" // [expect-error] Compile error at line 3 at '=': Invalid assignment target.
  }
}

Foo()
// [expect-error] Compile error at line 8 at end: Expecting '}' after block.