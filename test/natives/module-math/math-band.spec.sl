import Math

print Math // [expect] <Instance of Module>

// Basic bitwise AND
let a = Math.band(0, 0)
let b = Math.band(1, 0)
let c = Math.band(1, 1)

print a // [expect] 0
print b // [expect] 0
print c // [expect] 1

// More complex patterns
// 0b1010 (10) & 0b1100 (12) = 0b1000 (8)
a = Math.band(10, 12)
print a // [expect] 8

// 0b1111 (15) & 0b0101 (5) = 0b0101 (5)
b = Math.band(15, 5)
print b // [expect] 5

print a is Int // [expect] true
print b is Int // [expect] true
