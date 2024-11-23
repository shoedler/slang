cls Foo {
  ctor {         // [exit] 2
    ret "result" // [expect-error] Compile error at line 3 at 'ret': Can't return a value from a constructor.
  }
}
// [expect-error] Compile error at line 6 at end: Expecting '}' after block.