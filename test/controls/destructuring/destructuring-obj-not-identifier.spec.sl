// [exit] 2
let {true} = {true: 1} // [expect-error] Parser error at line 2 at 'true': Expecting identifier in destructuring assignment.
                       // [expect-error]      2 | let {true} = {true: 1}
                       // [expect-error]               ~~~~