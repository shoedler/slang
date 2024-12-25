// [exit] 2
let [a, ...] = [1, 2, 3, 4] // [expect-error] Parser error at line 2 at ']': Expecting identifier after ellipsis in destructuring.
                            // [expect-error]      2 | let [a, ...] = [1, 2, 3, 4]
                            // [expect-error]                     ~