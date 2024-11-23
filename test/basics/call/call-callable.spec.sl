// "Callable" means that the object can be called like a function.
fn foo -> 1
cls Bar { fn a -> 1 static fn b -> 2 }
let lol = fn -> 1
let bound = Bar().a
let unbound = Bar.b // Just a function

// Callables are Functions, Closures, Classes and Bound Methods.
print Bar() // [expect] <Instance of Bar>
print foo() // [expect] 1
print lol() // [expect] 1
print bound() // [expect] 1
print unbound() // [expect] 2

// But not every callable is a function.
print Bar is Fn // [expect] false
print foo is Fn // [expect] true
print lol is Fn // [expect] true
print bound is Fn // [expect] true
print unbound is Fn // [expect] true

// Since a class is callable, it can be passed as a method group.
print "12345".split("").map(Int) // [expect] [1, 2, 3, 4, 5]
