import Math

print Math // [expect] <Instance of Module>

let a = Math.ceil(0)
let b = Math.ceil(1)
let c = Math.ceil(-1)

print a // [expect] 0
print b // [expect] 1
print c // [expect] -1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

// Turns floats into integers
a = Math.ceil(0.0)
b = Math.ceil(1.0)
c = Math.ceil(-1.0)

print a // [expect] 0
print b // [expect] 1
print c // [expect] -1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

print Math.ceil(1.5) // [expect] 2
print Math.ceil(-1.5) // [expect] -1
print Math.ceil(1.99999999) // [expect] 2
print Math.ceil(1.00000001) // [expect] 2