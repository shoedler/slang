// Has "comparison" precedence, so preceeding comparisons get evaluated first
print 1 > 2 not in [true] // [expect] true

// Precedence is higher than equality operators
// --> 1 == (1 not in [true])
print  1 != 1  not in [true] // [expect] true
print (1 != 1) not in [true] // [expect] true

// Precedence is lower than arithmetic operators
// --> (1 + 2) not in [false]
print  1 + Int("2") not in [false] // [expect] true