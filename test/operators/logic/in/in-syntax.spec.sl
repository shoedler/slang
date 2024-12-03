// Works in types which have a 'has' method. Some examples are:
print 1 in [1,2,3] // [expect] true

print "foo" in { "foo": 1 } // [expect] true

print "oob" in "foobar" // [expect] true

// Instance methods are not considered, static methods are
cls X { fn x -> 1 static fn y -> 2 }
print "x" in X // [expect] false
print "y" in X // [expect] true

// See "has" tests for more examples

// Some implementations of 'has' also accept a fn that gets evaluated for each element
print (fn (x) -> x is Num) in [1,2,3] // [expect] true
// or
fn is_num(x) -> x is Num
print is_num in [1,2,3] // [expect] true

// Classes are callable, but are not functions - this allows easy type checking
const idk_what_type_i_am = nil
print typeof(idk_what_type_i_am) in [Str, Num, Float, Nil] // [expect] true
print typeof(idk_what_type_i_am) in [Str, Num, Float] // [expect] false

// Implementations of 'has' *should* return a boolean
cls Test { fn has(x) -> nil } // Custom 'has' implementation (returns nil)
// ...but they don't have to
print try (1 in Test()) else error // [expect] nil

