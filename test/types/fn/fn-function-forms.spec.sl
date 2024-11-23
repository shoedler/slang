// Anonymous Fn
let a = fn -> "a"
let a_args = fn(x,y,z) -> x + y + z

// Named Fn
fn b -> "b" 
fn b_args(x,y,z) -> x + y + z

print a      // [expect] <Fn (anon)>
print a_args // [expect] <Fn (anon)>
print b      // [expect] <Fn b>
print b_args // [expect] <Fn b_args>
print clock  // [expect] <NativeFn clock>
