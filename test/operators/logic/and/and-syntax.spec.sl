// Return the first non-true argument.
print false and 1 // [expect] false
print true and 1 // [expect] 1
print 1 and 2 and false // [expect] false

// Return the last argument if all are true.
print 1 and true // [expect] true
print 1 and 2 and 3 // [expect] 3

// Short-circuit at the first false argument.
let a = "before"
let b = "before"
let what = (a = true) and (b = false) and (a = "bad")

print a    // [expect] true
print b    // [expect] false
print what // [expect] false
