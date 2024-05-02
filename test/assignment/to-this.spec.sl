cls Foo {
  ctor {           // [Exit] 2
    this = "value" // [ExpectError] Compile error at line 3 at '=': Invalid assignment target.
  }
}

Foo()
// [ExpectError] Compile error at line 8 at end: Expecting '}' after block.