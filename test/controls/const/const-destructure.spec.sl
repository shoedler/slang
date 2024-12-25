// [exit] 2
const p = [1,2,3]

const [a,b,c] = p

a++ // [expect-error] Resolver error at line 6: Cannot assign to constant variable 'a'.
    // [expect-error]      6 | a++
    // [expect-error]          ~