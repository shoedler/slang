// Empty tuples are allowed, but it's a kinda weird syntax (kinda fitting, because it's also a weird thing to declare a tuple with no elements)
let a = (,)
print typeof(a) // [expect] <Class Tuple>
print a.len // [expect] 0

// Almost identically wierd is a tuple with only one element
let b = (1,)
print typeof(b) // [expect] <Class Tuple>
print b.len // [expect] 1

// Obviously, tuples with more than one element are more common

// Trailing commas are the reason why the syntax for tuples with one element or no elements is weird:
let c = (1, 2,) // Following the logic of the previous examples, this would be a tuple with 2 elements, but now the last comma is a trailing comma and thus ignored
print typeof(c) // [expect] <Class Tuple>
print c.len // [expect] 2