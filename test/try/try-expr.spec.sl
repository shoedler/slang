// Basic
let a = try 1 + true else "nope"
print a // [Expect] nope

// No else clause
let b = try 1 + true
print b // [Expect] nil

// Using the context variable
let c = try 1 + true else error
print c // [Expect] Operands must be two numbers or two strings. Left was Num, right was Bool.
print "still running" // [Expect] still running

// Nested 
print try try ASLDJKASLDJK else error else error // [Expect] Undefined variable 'ASLDJKASLDJK'.
print try try ASLDJKASLDJK else 1+nil else error // [Expect] Operands must be two numbers or two strings. Left was Num, right was Nil.

// Fuzzy tests
let d = (try fn -> 1 else fn -> 2)()
print d // [Expect] 1