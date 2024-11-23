let a = fn -> 1
fn b -> 2
cls Foo { fn c -> 3 static fn d -> 4}
let c = Foo().c // Bound method

print a.to_str()                    // [expect] <Fn (anon)>
print b.to_str()                    // [expect] <Fn b>
print try Foo.c.to_str() else error // [expect] Property 'c' does not exist on value of type Class.
print Foo.d.to_str()                // [expect] <Fn d>
print c.to_str()                    // [expect] <Fn c>

print a                    // [expect] <Fn (anon)>
print b                    // [expect] <Fn b>
print try Foo.c else error // [expect] Property 'c' does not exist on value of type Class.
print Foo.d                // [expect] <Fn d>
print c                    // [expect] <Fn c>
