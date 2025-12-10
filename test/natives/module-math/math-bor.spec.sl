import Math

print Math // [expect] <Instance of Module>

// Basic bitwise OR
let a = Math.bor(0, 0)
let b = Math.bor(1, 0)
let c = Math.bor(1, 1)

print a // [expect] 0
print b // [expect] 1
print c // [expect] 1

// More complex patterns
// 0b1010 (10) | 0b0101 (5) = 0b1111 (15)
a = Math.bor(10, 5)
print a // [expect] 15

// 0b11110000 (240) | 0b00001111 (15) = 0b11111111 (255)
b = Math.bor(240, 15)
print b // [expect] 255

print a is Int // [expect] true
print b is Int // [expect] true
