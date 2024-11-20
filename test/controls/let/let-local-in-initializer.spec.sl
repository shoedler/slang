let a = "outer"
{           // [Exit] 2
  let a = a // [ExpectError] Compile error at line 3 at 'a': Can't read local variable in its own initializer.
}
// [ExpectError] Compile error at line 5 at end: Expecting '}' after block.