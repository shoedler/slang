// Integers
print 123        // [expect] 123
print 987654     // [expect] 987654
print 0          // [expect] 0
print -0         // [expect] 0
print 123 is Int // [expect] true

// Floats
print 123.456      // [expect] 123.456
print -0.001       // [expect] -0.001
print 0.0 is Float // [expect] true

// Hex
print 0xDEADBEEF // [expect] 3735928559
print 0x12345678 // [expect] 305419896
print 0x1 is Int // [expect] true

// Binary
print 0b1010     // [expect] 10
print 0b1111     // [expect] 15
print 0b1 is Int // [expect] true

// Octal
print 0o777      // [expect] 511
print 0o1 is Int // [expect] true

// Int and Float are Num
print 1.5 is Num // [expect] true
print 0b1 is Num // [expect] true
