// [exit] 2
const p = [1,2,3]

const [a,b,c] = p

a++ // [expect-error] Compile error at line 6 at 'a': Can't reassign a constant.
