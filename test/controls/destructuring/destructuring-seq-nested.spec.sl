// [exit] 2
let [a, b, [c, d]]  = [1, 2, [3, 4]] // [expect-error] Parser error at line 2 at '[': Expecting identifier in destructuring assignment.
                                     // [expect-error]      2 | let [a, b, [c, d]]  = [1, 2, [3, 4]]
                                     // [expect-error]                     ~
                                     // [expect-error] Parser error at line 2 at ',': Unterminated destructuring pattern.
                                     // [expect-error]      2 | let [a, b, [c, d]]  = [1, 2, [3, 4]]
                                     // [expect-error]                   ~
                                     // [expect-error] Parser error at line 2 at '[': Expecting '=' after destructuring pattern.
                                     // [expect-error]      2 | let [a, b, [c, d]]  = [1, 2, [3, 4]]
                                     // [expect-error]                     ~
                                     // [expect-error] Parser error at line 2 at ']': Expecting expression.
                                     // [expect-error]      2 | let [a, b, [c, d]]  = [1, 2, [3, 4]]
                                     // [expect-error]                           ~