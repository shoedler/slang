{
  cls Foo : Foo {} // [ExpectCompileError]  ERROR at [line 2] at 'Foo': A class can't inherit from itself.
}
// [ExpectCompileError] ERROR at [line 4] at end: Expecting '}' after block.