let a = "a"
let b = "b"
let c = "c"

// Assignment is right-associative.
a = b = c
print a // [Expect] c
print b // [Expect] c
print c // [Expect] c
