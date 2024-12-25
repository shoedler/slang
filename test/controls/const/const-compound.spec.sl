// [exit] 2
const a = 1
let b = a++ // [expect-error] Resolver error at line 3: Cannot assign to constant variable 'a'.
            // [expect-error]      3 | let b = a++
            // [expect-error]                  ~