// Has "comparison" precedence, so preceeding comparisons get evaluated first
print 1 > 2 in [true] // [expect] false

// Precedence is higher than equality operators
// --> 1 == (1 in [true])
print  1 == 1  in [true] // [expect] false
print (1 == 1) in [true] // [expect] true

// Precedence is lower than arithmetic operators
// --> (1 + 2) in [true]
print  1 + Int("2") in [true] // [expect] false