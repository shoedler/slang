// [exit] 2
let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}} // [expect-error] Compile error at line 2 at '{': Expecting identifier in destructuring assignment.