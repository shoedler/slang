// [exit] 2
// [expect-error] Parser error at line 5 at 'b': Rest parameter must be last in destructuring assignment.
// [expect-error]      5 | let [a, ...b, c] = [1, 2, 3, 4]
// [expect-error]                     ~
let [a, ...b, c] = [1, 2, 3, 4] 