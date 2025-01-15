// [exit] 2
let a = {cls X {}: cls Y {}} // [expect-error] Parser error at line 2 at 'cls': Expecting expression.
                             // [expect-error]      2 | let a = {cls X {}: cls Y {}}
                             // [expect-error]                   ~~~
                             // [expect-error] Parser error at line 2 at '}': Expecting expression.
                             // [expect-error]      2 | let a = {cls X {}: cls Y {}}
                             // [expect-error]                                     ~