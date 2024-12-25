// [exit] 2
for let a = 1; cls x {}; a++; {} // [expect-error] Parser error at line 2 at 'cls': Expecting expression.
                                 // [expect-error]      2 | for let a = 1; cls x {}; a++; {}
                                 // [expect-error]                         ~~~
                                 // [expect-error] Parser error at line 2 at 'x': Expecting ';' after loop condition.
                                 // [expect-error]      2 | for let a = 1; cls x {}; a++; {}
                                 // [expect-error]                             ~
                                 // [expect-error] Parser error at line 2 at '{': Expecting ';' after loop increment.
                                 // [expect-error]      2 | for let a = 1; cls x {}; a++; {}
                                 // [expect-error]                               ~
                                 // [expect-error] Parser error at line 2 at ';': Expecting expression.
                                 // [expect-error]      2 | for let a = 1; cls x {}; a++; {}
                                 // [expect-error]                                 ~
                                 // [expect-error] Parser error at line 2 at ';': Expecting expression.
                                 // [expect-error]      2 | for let a = 1; cls x {}; a++; {}
                                 // [expect-error]                                      ~