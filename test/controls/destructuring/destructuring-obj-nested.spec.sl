// [exit] 2
let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}} // [expect-error] Parser error at line 2 at '{': Nested Obj destructuring is not allowed.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                     ~