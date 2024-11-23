// [exit] 2
{
  cls Foo : Foo {} // [expect-error] Compile error at line 3 at 'Foo': A class can't inherit from itself.
}
// [expect-error] Compile error at line 5 at end: Expecting '}' after block.