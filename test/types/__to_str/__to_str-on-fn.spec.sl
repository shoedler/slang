let a = fn -> 1
fn b -> 2
cls Foo { fn c -> 3 static fn d -> 4}
let c = Foo().c // Bound method

print a.to_str()                    // [Expect] <Fn (anon)>
print b.to_str()                    // [Expect] <Fn b>
print try Foo.c.to_str() else error // [Expect] Property 'c' does not exist on value of type Class.
print Foo.d.to_str()                // [Expect] <Fn d>
print c.to_str()                    // [Expect] <Fn c>

print a                    // [Expect] <Fn (anon)>
print b                    // [Expect] <Fn b>
print try Foo.c else error // [Expect] Property 'c' does not exist on value of type Class.
print Foo.d                // [Expect] <Fn d>
print c                    // [Expect] <Fn c>
