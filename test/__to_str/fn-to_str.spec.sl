let a = fn -> 1
fn b -> 2
cls Foo { fn c -> 3}
let c = Foo().c // Bound method

print a.to_str()     // [Expect] <Fn __anon>
print b.to_str()     // [Expect] <Fn b>
print Foo.c.to_str() // [Expect] <Fn c>
print c.to_str()     // [Expect] <Fn c>

print a // [Expect] <Fn __anon>
print b // [Expect] <Fn b>
print Foo.c // [Expect] <Fn c>
print c // [Expect] <Fn c>
