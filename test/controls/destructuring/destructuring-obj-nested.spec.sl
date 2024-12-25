// [exit] 2
let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}} // [expect-error] Parser error at line 2 at '{': Expecting identifier in destructuring assignment.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                     ~
                                                         // [expect-error] Parser error at line 2 at ',': Unterminated destructuring pattern.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                   ~
                                                         // [expect-error] Parser error at line 2 at '{': Expecting '=' after destructuring pattern.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                     ~
                                                         // [expect-error] Parser error at line 2 at ',': Expecting ':' after key.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                       ~
                                                         // [expect-error] Parser error at line 2 at ',': Expecting expression.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                       ~
                                                         // [expect-error] Parser error at line 2 at 'd': Expecting '}' after Obj literal. Or maybe you are missing a ','?
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                         ~
                                                         // [expect-error] Parser error at line 2 at '}': Expecting expression.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                          ~
                                                         // [expect-error] Parser error at line 2 at '}': Expecting expression.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                           ~
                                                         // [expect-error] Parser error at line 2 at '}': Expecting ':' after key.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                                                                 ~
                                                         // [expect-error] Parser error at line 2 at '}': Expecting expression.
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error]                                                                 ~
                                                         // [expect-error] Parser error at line 2 at end: Expecting '}' after Obj literal. Or maybe you are missing a ','?
                                                         // [expect-error]      2 | let {a, b, {c, d}}  = {"a": 1, "b": 2, {"c": 3, "d": 4}}
                                                         // [expect-error] 