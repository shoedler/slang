// Works in types which have a 'has' method. Some examples are:
print 1 not in [1,2,3] // [expect] false
print 2 not in [1,2,3] // [expect] false
print 4 not in [1,2,3] // [expect] true

print "foo" not in { "foo": 1 } // [expect] false

print "oob" not in "foobar" // [expect] false

// Instance methods are not considered, static methods are
cls X { fn x -> 1 static fn y -> 2 }
print "x" not in X // [expect] true
print "y" not in X // [expect] false

// See "has" tests for more examples

// Some implementations of 'has' also accept a fn that gets evaluated for each element
print (fn (x) -> x is Num) not in [1,2,3] // [expect] false
// or
fn is_num(x) -> x is Num
print is_num not in [1,2,3] // [expect] false
print is_num not in [1,"two",3.0] // [expect] false
print is_num not in ["one","two","three"] // [expect] true

// Classes are callable, but are not functions - this allows easy type checking
const idk_what_type_i_am = nil
print typeof(idk_what_type_i_am) not in [Str, Num, Float, Nil] // [expect] false
print typeof(idk_what_type_i_am) not in [Str, Num, Float] // [expect] true

// Implementations of 'has' *should* return a boolean
cls Test { fn has(x) -> nil } // Custom 'has' implementation (returns nil)
// ...but they don't have to
print try (1 not in Test()) else error // [expect] true
//                                                 ^~~^ true, because "not" nil is true - losing "not" would yield nil

