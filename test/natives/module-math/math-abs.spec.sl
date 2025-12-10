import Math

print Math // [expect] <Instance of Module>

let a = Math.abs(0)
let b = Math.abs(1)
let c = Math.abs(-1)

print a // [expect] 0
print b // [expect] 1
print c // [expect] 1

print a is Int // [expect] true
print b is Int // [expect] true
print c is Int // [expect] true

a = Math.abs(0.0)
b = Math.abs(1.0)
c = Math.abs(-1.0)

print a // [expect] 0
print b // [expect] 1
print c // [expect] 1

print a is Float // [expect] true
print b is Float // [expect] true
print c is Float // [expect] true
