{
  cls Foo : Foo {} // [ExpectCompileError]  Compile error at line 2 at 'Foo': A class can't inherit from itself.
}
// [ExpectCompileError] Compile error at line 4 at end: Expecting '}' after block.