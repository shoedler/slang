import Math

print Math // [expect] <Instance of Module>

let a = Math.floor(0)
let b = Math.floor(1)
let c = Math.floor(-1)

print a // [expect] 0
print b // [expect] 1
print c // [expect] -1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

// Turns floats into integers
a = Math.floor(0.0)
b = Math.floor(1.0)
c = Math.floor(-1.0)

print a // [expect] 0
print b // [expect] 1
print c // [expect] -1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

print Math.floor(1.5) // [expect] 1
print Math.floor(-1.5) // [expect] -2
print Math.floor(1.99999999) // [expect] 1