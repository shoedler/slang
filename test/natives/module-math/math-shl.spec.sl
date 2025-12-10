import Math

print Math // [expect] <Instance of Module>

// Shift-left identity & simple shifts
let a = Math.shl(1, 0) // 1 << 0
let b = Math.shl(1, 1) // 1 << 1
let c = Math.shl(1, 4) // 1 << 4
let d = Math.shl(3, 2) // 3 (0b0011) << 2 = 0b1100 (12)

print a // [expect] 1
print b // [expect] 2
print c // [expect] 16
print d // [expect] 12

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true
print d is Int // [expect] true
