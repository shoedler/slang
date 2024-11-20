// Return the first non-true argument.
print false and 1 // [Expect] false
print true and 1 // [Expect] 1
print 1 and 2 and false // [Expect] false

// Return the last argument if all are true.
print 1 and true // [Expect] true
print 1 and 2 and 3 // [Expect] 3

// Short-circuit at the first false argument.
let a = "before"
let b = "before"
let what = (a = true) and (b = false) and (a = "bad")

print a    // [Expect] true
print b    // [Expect] false
print what // [Expect] false
