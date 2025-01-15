let a = "a"
let b = "b"     // [exit] 2
a + b = "value" // [expect-error] Parser error at line 3 at '=': Invalid assignment target.
                // [expect-error]      3 | a + b = "value"
                // [expect-error]                ~