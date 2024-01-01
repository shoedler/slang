// Anonymous Fn
let a = fn -> "a"
let a_args = fn(x,y,z) -> x + y + z

// Named Fn
fn b -> "b" 
fn b_args(x,y,z) -> x + y + z

print a      // [Expect] [Fn <Anon>, arity 0]
print a_args // [Expect] [Fn <Anon>, arity 3]
print b      // [Expect] [Fn b, arity 0]
print b_args // [Expect] [Fn b_args, arity 3]
print clock  // [Expect] [Native Fn]
