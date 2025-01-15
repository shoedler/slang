// An empty tuple is a valid expression, but it's not written like this:
// [exit] 2
let a = () 
// [expect-error] Parser error at line 3 at ')': Expecting expression.
// [expect-error]      3 | let a = ()
// [expect-error]                   ~