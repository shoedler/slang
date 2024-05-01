// [Exit] 2
{
  cls Foo : Foo {} // [ExpectError] Compile error at line 3 at 'Foo': A class can't inherit from itself.
}
// [ExpectError] Compile error at line 4 at end: Expecting '}' after block.