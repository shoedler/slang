 let a = 123  // [exit] 2
(a) = "value" // [expect-error] Parser error at line 2 at '=': Invalid assignment target.
              // [expect-error]      2 | (a) = "value"
              // [expect-error]              ~