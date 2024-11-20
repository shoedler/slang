// Integers
print 123        // [Expect] 123
print 987654     // [Expect] 987654
print 0          // [Expect] 0
print -0         // [Expect] 0
print 123 is Int // [Expect] true

// Floats
print 123.456      // [Expect] 123.456
print -0.001       // [Expect] -0.001
print 0.0 is Float // [Expect] true

// Hex
print 0xDEADBEEF // [Expect] 3735928559
print 0x12345678 // [Expect] 305419896
print 0x1 is Int // [Expect] true

// Binary
print 0b1010     // [Expect] 10
print 0b1111     // [Expect] 15
print 0b1 is Int // [Expect] true

// Octal
print 0o777      // [Expect] 511
print 0o1 is Int // [Expect] true

// Int and Float are Num
print 1.5 is Num // [Expect] true
print 0b1 is Num // [Expect] true
