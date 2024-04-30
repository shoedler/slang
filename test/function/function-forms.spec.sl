// Anonymous Fn
let a = fn -> "a"
let a_args = fn(x,y,z) -> x + y + z

// Named Fn
fn b -> "b" 
fn b_args(x,y,z) -> x + y + z

print a      // [Expect] <Fn (anon)>
print a_args // [Expect] <Fn (anon)>
print b      // [Expect] <Fn b>
print b_args // [Expect] <Fn b_args>
print clock  // [Expect] <NativeFn clock>
