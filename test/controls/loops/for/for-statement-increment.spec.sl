// [exit] 2
for ; let a = 1; a < 2; {}; {} // [expect-error] Parser error at line 2 at 'let': Expecting expression.
                               // [expect-error]      2 | for ; let a = 1; a < 2; {}; {}
                               // [expect-error]                ~~~