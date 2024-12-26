// [exit] 2
const r = r // [expect-error] Resolver error at line 2: Cannot read variable in its own initializer.
            // [expect-error]      2 | const r = r
            // [expect-error]                    ~