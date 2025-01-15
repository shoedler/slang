// [exit] 2
if false let bar // [expect-error] Parser error at line 2 at 'let': Expecting expression.
if true let foo  // [expect-error]      2 | if false let bar
                 // [expect-error]                   ~~~
                 // [expect-error] Parser error at line 3 at 'let': Expecting expression.
                 // [expect-error]      3 | if true let foo
                 // [expect-error]                  ~~~