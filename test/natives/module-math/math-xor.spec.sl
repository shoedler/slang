import Math

print Math // [expect] <Instance of Module>

// Basic small values
let a = Math.xor(0, 0)
let b = Math.xor(0, 1)
let c = Math.xor(1, 1)

print a // [expect] 0
print b // [expect] 1
print c // [expect] 0

// Larger bit patterns (using decimal)
a = Math.xor(15, 0)      // 0b1111 ^ 0b0000
b = Math.xor(15, 15)     // 0b1111 ^ 0b1111
c = Math.xor(240, 15)    // 0b11110000 ^ 0b00001111 = 0b11111111 = 255

print a // [expect] 15
print b // [expect] 0
print c // [expect] 255

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true
