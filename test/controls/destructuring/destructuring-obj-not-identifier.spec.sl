// [exit] 2
let {true} = {true: 1} // [expect-error] Parser error at line 2 at 'true': Expecting identifier in destructuring assignment.
                       // [expect-error]      2 | let {true} = {true: 1}
                       // [expect-error]               ~~~~
                       // [expect-error] Parser error at line 2 at '{': Unterminated destructuring pattern.
                       // [expect-error]      2 | let {true} = {true: 1}
                       // [expect-error]              ~
                       // [expect-error] Parser error at line 2 at 'true': Expecting '=' after destructuring pattern.
                       // [expect-error]      2 | let {true} = {true: 1}
                       // [expect-error]               ~~~~
                       // [expect-error] Parser error at line 2 at '}': Expecting expression.
                       // [expect-error]      2 | let {true} = {true: 1}
                       // [expect-error]                   ~