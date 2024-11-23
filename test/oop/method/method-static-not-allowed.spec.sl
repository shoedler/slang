fn x {           // [exit] 2
  let static = 1 // [expect-error] Compile error at line 2 at 'static': Expecting variable name.
  print static   // [expect-error] Compile error at line 3 at 'static': Expecting expression.
}                // [expect-error] Compile error at line 4 at end: Expecting '}' after block.