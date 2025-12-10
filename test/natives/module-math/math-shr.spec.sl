import Math

print Math // [expect] <Instance of Module>

// Shift-right identity & simple shifts (non-negative)
let a = Math.shr(1, 0) // 1 >> 0
let b = Math.shr(4, 1) // 4 >> 1
let c = Math.shr(4, 2) // 4 >> 2
let d = Math.shr(5, 2) // 5 (0b0101) >> 2 = 0b0001

print a // [expect] 1
print b // [expect] 2
print c // [expect] 1
print d // [expect] 1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true
print d is Int // [expect] true
