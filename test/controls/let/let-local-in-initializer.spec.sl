let a = "outer"
{           // [exit] 2
  let a = a // [expect-error] Compile error at line 3 at 'a': Can't read local variable in its own initializer.
}
// [expect-error] Compile error at line 5 at end: Expecting '}' after block.