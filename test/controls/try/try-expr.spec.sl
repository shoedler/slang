// Basic
let a = try 1 + true else "nope"
print a // [expect] nope

// No else clause
let b = try 1 + true
print b // [expect] nil

// Using the context variable
let c = try 1 + true else error
print c // [expect] Incompatible types for binary operand +. Left was Int, right was Bool.
print "still running" // [expect] still running

// Nested 
print try try ASLDJKASLDJK else error else error // [expect] Undefined variable 'ASLDJKASLDJK'.
print try try ASLDJKASLDJK else 1+nil else error // [expect] Incompatible types for binary operand +. Left was Int, right was Nil.

// Fuzzy tests
let d = (try fn -> 1 else fn -> 2)()
print d // [expect] 1