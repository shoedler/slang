fn x {
  let static = 1 // [ExpectCompileError] Compile error at line 2 at 'static': Expecting variable name.
  print static   // [ExpectCompileError] Compile error at line 3 at 'static': Expecting expression.
}                // [ExpectCompileError] Compile error at line 4 at end: Expecting '}' after block.