let a = "a"          // [exit] 2
let f = !a = "value" // [expect-error] Parser error at line 2 at '=': Invalid assignment target.
                     // [expect-error]      2 | let f = !a = "value"
                     // [expect-error]                     ~