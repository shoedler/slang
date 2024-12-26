// [exit] 2
for ;; let foo; // [expect-error] Parser error at line 2 at 'let': Expecting expression.
                // [expect-error]      2 | for ;; let foo;
                // [expect-error]                 ~~~