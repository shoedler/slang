// [exit] 2
let (a, ...b, c) = (1, 2, 3, 4) // [expect-error] Parser error at line 2 at 'b': Rest parameter must be last in destructuring assignment.
                                // [expect-error]      2 | let (a, ...b, c) = (1, 2, 3, 4)
                                // [expect-error]                     ~