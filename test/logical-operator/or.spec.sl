// Note: These tests implicitly depend on ints being truthy.

// Return the first true argument.
print 1 or true // [Expect] 1
print false or 1 // [Expect] 1
print false or false or true // [Expect] true

// Return the last argument if all are false.
print false or false // [Expect] false
print false or false or false // [Expect] false

// Short-circuit at the first true argument.
let a = "before"
let b = "before"
let what = (a = false) or (b = true) or (a = "bad")

print a    // [Expect] false
print b    // [Expect] true
print what // [Expect] true