let a = "outer"
{
  let a = a // [ExpectCompileError] Compile error at line 3 at 'a': Can't read local variable in its own initializer.
}
// [ExpectCompileError] Compile error at line 5 at end: Expecting '}' after block.