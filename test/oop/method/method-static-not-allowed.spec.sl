fn x {           // [Exit] 2
  let static = 1 // [ExpectError] Compile error at line 2 at 'static': Expecting variable name.
  print static   // [ExpectError] Compile error at line 3 at 'static': Expecting expression.
}                // [ExpectError] Compile error at line 4 at end: Expecting '}' after block.