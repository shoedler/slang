// Works in types which have a 'has' method. Some examples are:
print 1 in [1,2,3] // [Expect] true

print "foo" in { "foo": 1 } // [Expect] true

print "oob" in "foobar" // [Expect] true

cls X { fn x -> 1 static fn y -> 2 }
print "x" in X // [Expect] true
print "y" in X // [Expect] true

// See "has" tests for more examples

// Fails if the right-hand side is not a type that supports 'has'
print try (1 in 1) else error // [Expect] Type Num does not support 'has'.

// Some implementations of 'has' also accept a callable that gets evaluated for each element
print (fn (x) -> x is Num) in [1,2,3] // [Expect] true
// or
fn is_num(x) -> x is Num
print is_num in [1,2,3] // [Expect] true

// Implementations of 'has' should return a boolean
cls Test { fn has(x) -> nil } // Custom 'has' implementation (returns nil)
print try (1 in Test()) else error // [Expect] Method 'has' on type Test must return a Bool, but got Nil.
