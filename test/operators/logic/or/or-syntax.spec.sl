// Return the first true argument.
print 1 or true // [expect] 1
print false or 1 // [expect] 1
print false or false or true // [expect] true

// Return the last argument if all are false.
print false or false // [expect] false
print false or false or false // [expect] false

// Short-circuit at the first true argument.
let a = "before"
let b = "before"
let what = (a = false) or (b = true) or (a = "bad")

print a    // [expect] false
print b    // [expect] true
print what // [expect] true
