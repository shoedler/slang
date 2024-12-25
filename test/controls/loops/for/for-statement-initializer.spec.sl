// [exit] 2
for ;{}; a < 2; a++; {} // [expect-error] Parser error at line 2 at ';': Expecting expression.
                        // [expect-error]      2 | for ;{}; a < 2; a++; {}
                        // [expect-error]                             ~