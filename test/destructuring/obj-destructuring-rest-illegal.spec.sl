let {a, ...b} = {"a": 1, "b": 2, "c": 3, "d": 4} // [ExpectCompileError] Compile error at line 1 at '...': Rest parameter is not allowed in Obj destructuring assignment.