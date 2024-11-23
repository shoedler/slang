// [exit] 2
fn ctor { // [expect-error] Compile error at line 2 at 'ctor': Expecting function name.
  print "not constructor"
} // [expect-error] Compile error at line 4 at '}': Expecting expression.