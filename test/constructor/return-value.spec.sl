cls Foo {
  ctor {         // [Exit] 2
    ret "result" // [ExpectError] Compile error at line 3 at 'ret': Can't return a value from a constructor.
  }
}
// [ExpectError] Compile error at line 6 at end: Expecting '}' after block.