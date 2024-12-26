// [exit] 2
let {a} = {a:1} // [expect-error] Resolver error at line 2: Cannot read variable in its own initializer.
                // [expect-error]      2 | let {a} = {a:1}
                // [expect-error]                     ~