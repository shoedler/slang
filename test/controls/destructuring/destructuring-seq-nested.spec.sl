// [exit] 2
// [expect-error] Parser error at line 5 at '[': Expecting identifier in destructuring assignment.
// [expect-error]      5 | let [a, b, [c, d]]  = [1, 2, [3, 4]]
// [expect-error]                     ~
let [a, b, [c, d]]  = [1, 2, [3, 4]]