
import Math

print Math // [expect] <Instance of Module>

let a = Math.round(0)
let b = Math.round(1)
let c = Math.round(-1)

print a // [expect] 0
print b // [expect] 1
print c // [expect] -1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

// Turns floats into integers
a = Math.round(0.0)
b = Math.round(1.0)
c = Math.round(-1.0)

print a // [expect] 0
print b // [expect] 1
print c // [expect] -1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

// Standard rounding behavior
print Math.round(0.1) // [expect] 0
print Math.round(0.49) // [expect] 0
print Math.round(0.5) // [expect] 1
print Math.round(0.51) // [expect] 1

print Math.round(1.4) // [expect] 1
print Math.round(1.5) // [expect] 2
print Math.round(1.6) // [expect] 2

// Negative values
print Math.round(-0.1) // [expect] 0
print Math.round(-0.49) // [expect] 0
print Math.round(-0.5) // [expect] -1
print Math.round(-0.51) // [expect] -1

print Math.round(-1.4) // [expect] -1
print Math.round(-1.5) // [expect] -2
print Math.round(-1.6) // [expect] -2

// Floating point precision edge cases
print Math.round(1.00000001) // [expect] 1
print Math.round(1.49999999) // [expect] 1
print Math.round(1.50000001) // [expect] 2

// Large values
print Math.round(1000000.4) // [expect] 1000000
print Math.round(1000000.5) // [expect] 1000001

// Result type must always be Int
print Math.round(0.5) is Int // [expect] true
print Math.round(-0.5) is Int // [expect] true
print Math.round(123456789.5) is Int // [expect] true
