// Integers
print 123     // [Expect] 123
print 987654  // [Expect] 987654
print 0       // [Expect] 0
print -0      // [Expect] 0

// Floats
print 123.456 // [Expect] 123.456
print -0.001  // [Expect] -0.001

// Hex
print 0xDEADBEEF // [Expect] 3735928559
print 0x12345678 // [Expect] 305419896

// Binary
print 0b1010 // [Expect] 10
print 0b1111 // [Expect] 15

// Octal
print 0o777 // [Expect] 511