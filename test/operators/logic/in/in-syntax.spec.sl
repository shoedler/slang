// Works in types which have a 'has' method. Some examples are:
print 1 in [1,2,3] // [expect] true

print "foo" in { "foo": 1 } // [expect] true

print "oob" in "foobar" // [expect] true

// Instance methods are not considered
cls X { fn x -> 1 static fn y -> 2 }
print "x" in X // [expect] false
print "y" in X // [expect] true

// See "has" tests for more examples

// Some implementations of 'has' also accept a callable that gets evaluated for each element
print (fn (x) -> x is Num) in [1,2,3] // [expect] true
// or
fn is_num(x) -> x is Num
print is_num in [1,2,3] // [expect] true

// Implementations of 'has' should return a boolean
cls Test { fn has(x) -> nil } // Custom 'has' implementation (returns nil)
print try (1 in Test()) else error // [expect] Method 'has' on type Test must return a Bool, but got Nil.
