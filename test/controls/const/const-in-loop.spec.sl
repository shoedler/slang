// [exit] 2
for const i = 0; i < 10; i++; { // [expect-error] Compile error at line 2 at 'const': Expecting expression.
  print i.to_str()              // [expect-error] Compile error at line 4 at '}': Expecting expression.
}

// Fails because we check for LET in the compiler, which is also ok.
