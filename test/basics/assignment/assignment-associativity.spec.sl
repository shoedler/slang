let a = "a"
let b = "b"
let c = "c"

// Assignment is right-associative.
a = b = c
print a // [expect] c
print b // [expect] c
print c // [expect] c
