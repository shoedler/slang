// [exit] 2
let {a, b,  = {"a": 1, "b": 2, "c": 3} // [expect-error] Parser error at line 2 at '=': Expecting identifier in destructuring assignment.
                                       // [expect-error]      2 | let {a, b,  = {"a": 1, "b": 2, "c": 3}
                                       // [expect-error]                      ~