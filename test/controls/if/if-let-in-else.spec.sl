// [exit] 2
if true "ok" else let foo // [expect-error] Parser error at line 2 at 'let': Expecting expression.
                          // [expect-error]      2 | if true "ok" else let foo
                          // [expect-error]                            ~~~