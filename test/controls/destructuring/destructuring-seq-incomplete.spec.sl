// [exit] 2
let [a, b,  = [1, 2, 3] // [expect-error] Parser error at line 2 at '=': Expecting identifier in destructuring assignment.
                        // [expect-error]      2 | let [a, b,  = [1, 2, 3]
                        // [expect-error]                      ~