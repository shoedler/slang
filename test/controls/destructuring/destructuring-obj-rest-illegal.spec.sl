// [exit] 2
let {a, ...b} = {"a": 1, "b": 2, "c": 3, "d": 4} // [expect-error] Parser error at line 2 at '...': Rest parameter is not allowed in Obj destructuring assignment.
                                                 // [expect-error]      2 | let {a, ...b} = {"a": 1, "b": 2, "c": 3, "d": 4}
                                                 // [expect-error]                  ~~~