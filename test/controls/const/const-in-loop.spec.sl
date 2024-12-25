// [exit] 2
for const i = 0; i < 10; i++; { // [expect-error] Parser error at line 2 at 'const': Expecting expression.
  print i.to_str()              // [expect-error]      2 | for const i = 0; i < 10; i++; {
}                               // [expect-error]              ~~~~~
                                // [expect-error] Parser error at line 2 at 'i': Expecting ';' after loop initializer.
                                // [expect-error]      2 | for const i = 0; i < 10; i++; {
                                // [expect-error]                    ~
                                // [expect-error] Parser error at line 2 at ';': Expecting expression.
                                // [expect-error]      2 | for const i = 0; i < 10; i++; {
                                // [expect-error]                                      ~