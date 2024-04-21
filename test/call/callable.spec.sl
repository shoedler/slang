// "Callable" means that the object can be called like a function.
fn foo -> 1
cls Bar { fn a -> 1 static fn b -> 2 }
let lol = fn -> 1
let bound = Bar().a
let unbound = Bar.b // Just a function

// Callables are Functions, Closures, Classes and Bound Methods.
print Bar() // [Expect] <Instance of Bar>
print foo() // [Expect] 1
print lol() // [Expect] 1
print bound() // [Expect] 1
print unbound() // [Expect] 2

// But not every callable is a function.
print Bar is Fn // [Expect] false
print foo is Fn // [Expect] true
print lol is Fn // [Expect] true
print bound is Fn // [Expect] true
print unbound is Fn // [Expect] true


