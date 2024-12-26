// [exit] 2
fn ctor {                 // [expect-error] Parser error at line 2 at 'ctor': Expecting function name.
  print "not constructor" // [expect-error]      2 | fn ctor {
}                         // [expect-error]             ~~~~
                          // [expect-error] Parser error at line 4 at '}': Expecting expression.
                          // [expect-error]      4 | }
                          // [expect-error]          ~