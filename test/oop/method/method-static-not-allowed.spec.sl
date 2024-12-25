fn x {           // [exit] 2
  let static = 1 // [expect-error] Parser error at line 2 at 'static': Expecting variable name.
  print static   // [expect-error]      2 |   let static = 1
}                // [expect-error]                ~~~~~~