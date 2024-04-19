fn x {
  let {a} = {a:1} // [ExpectCompileError] Compile error at line 2 at 'a': Can't read local variable in its own initializer.
} // [ExpectCompileError] Compile error at line 3 at end: Expecting '}' after block.