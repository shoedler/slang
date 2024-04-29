// Tuples are equal if their elements are equal (in order)
let a = (1, 2, 3)
let b = (1, 2, 3)
let c = (3, 2, 1)

print a == b // [Expect] true
print a == c // [Expect] false

// Even if you have objects in them. Obviously, they must be the same reference
let x = { "a": 1 }
a = (x, 2, 3)
b = (x, 2, 3)

print a == b // [Expect] true

// Nested tuples behave the same way
a = (1, (2, 3))
b = (1, (2, 3))
c = (1, (3, 2))

print a == b // [Expect] true
print a == c // [Expect] false
