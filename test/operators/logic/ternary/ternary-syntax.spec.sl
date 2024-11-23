let a = 123
let b = "Hi"
let c
let d = nil

print a ? "a is Truthy!" : "a is Falsy!" // [expect] a is Truthy!
print b ? "b is Truthy!" : "b is Falsy!" // [expect] b is Truthy!
print c ? "c is Truthy!" : "c is Falsy!" // [expect] c is Falsy!
print d ? "d is Truthy!" : "d is Falsy!" // [expect] d is Falsy!
