print 1.to_str()          // [expect] 1
print 1.0.to_str()        // [expect] 1
print 1.1.to_str()        // [expect] 1.1
print (-1).to_str()       // [expect] -1
print (-1.0).to_str()     // [expect] -1
print (-1.1).to_str()     // [expect] -1.1
print 0b101.to_str()      // [expect] 5
print 0xDEADBEEF.to_str() // [expect] 3735928559
print 0o777.to_str()      // [expect] 511

print 1          // [expect] 1
print 1.0        // [expect] 1
print 1.1        // [expect] 1.1
print -1         // [expect] -1
print -1.0       // [expect] -1
print -1.1       // [expect] -1.1
print 0b101      // [expect] 5
print 0xDEADBEEF // [expect] 3735928559
print 0o777      // [expect] 511

// Precision is kinda whack currently:
print 0.000000000000000001 // [expect] 0
print 0.00000000000000001 // [expect] 0
print 0.0000000000000001 // [expect] 0
print 0.000000000000001 // [expect] 0
print 0.00000000000001 // [expect] 0.00000000000001
print 0.0000000000001 // [expect] 0.0000000000001
print 0.000000000001 // [expect] 0.000000000001
print 0.00000000001 // [expect] 0.00000000001
print 0.0000000001 // [expect] 0.0000000001
print 0.000000001 // [expect] 0.000000001
print 0.00000001 // [expect] 0.00000001
print 0.0000001 // [expect] 0.0000001
print 0.000001 // [expect] 0.000001
print 0.00001 // [expect] 0.00001
print 0.0001 // [expect] 0.0001
print 0.001 // [expect] 0.001
print 0.01 // [expect] 0.01
print 0.1 // [expect] 0.1
print 1 // [expect] 1

print 0.00000000000 // [expect] 0
