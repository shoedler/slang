import Math

print Math // [expect] <Instance of Module>

// Integer base & exponent -> Int
let a = Math.pow(2, 0)
let b = Math.pow(2, 3)
let c = Math.pow(-2, 3)
let d = Math.pow(-2, 2)

print a // [expect] 1
print b // [expect] 8
print c // [expect] -8
print d // [expect] 4

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true
print d is Int // [expect] true

// Float base & exponent -> Float
a = Math.pow(2.0, 0.0)
b = Math.pow(2.0, 3.0)
c = Math.pow(9.0, 0.5)
d = Math.pow(2.0, -1.0)

print a // [expect] 1
print b // [expect] 8
print c // [expect] 3
print d // [expect] 0.5

print a is Float // [expect] true
print b is Float // [expect] true
print c is Float // [expect] true
print d is Float // [expect] true

// Non-integer exponents
print Math.pow(1.5, 2.0) // [expect] 2.25
print Math.pow(4.0, 0.5) // [expect] 2
