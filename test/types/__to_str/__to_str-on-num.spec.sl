print 1.to_str()          // [Expect] 1
print 1.0.to_str()        // [Expect] 1
print 1.1.to_str()        // [Expect] 1.1
print (-1).to_str()       // [Expect] -1
print (-1.0).to_str()     // [Expect] -1
print (-1.1).to_str()     // [Expect] -1.1
print 0b101.to_str()      // [Expect] 5
print 0xDEADBEEF.to_str() // [Expect] 3735928559
print 0o777.to_str()      // [Expect] 511

print 1          // [Expect] 1
print 1.0        // [Expect] 1
print 1.1        // [Expect] 1.1
print -1         // [Expect] -1
print -1.0       // [Expect] -1
print -1.1       // [Expect] -1.1
print 0b101      // [Expect] 5
print 0xDEADBEEF // [Expect] 3735928559
print 0o777      // [Expect] 511

// Precision is kinda whack currently:
print 0.000000000000000001 // [Expect] 0
print 0.00000000000000001 // [Expect] 0
print 0.0000000000000001 // [Expect] 0
print 0.000000000000001 // [Expect] 0
print 0.00000000000001 // [Expect] 0.00000000000001
print 0.0000000000001 // [Expect] 0.0000000000001
print 0.000000000001 // [Expect] 0.000000000001
print 0.00000000001 // [Expect] 0.00000000001
print 0.0000000001 // [Expect] 0.0000000001
print 0.000000001 // [Expect] 0.000000001
print 0.00000001 // [Expect] 0.00000001
print 0.0000001 // [Expect] 0.0000001
print 0.000001 // [Expect] 0.000001
print 0.00001 // [Expect] 0.00001
print 0.0001 // [Expect] 0.0001
print 0.001 // [Expect] 0.001
print 0.01 // [Expect] 0.01
print 0.1 // [Expect] 0.1
print 1 // [Expect] 1

print 0.00000000000 // [Expect] 0
