let a = 123
let b = "Hi"
let c
let d = nil

print a ? "a is Truthy!" : "a is Falsy!" // [Expect] a is Truthy!
print b ? "b is Truthy!" : "b is Falsy!" // [Expect] b is Truthy!
print c ? "c is Truthy!" : "c is Falsy!" // [Expect] c is Falsy!
print d ? "d is Truthy!" : "d is Falsy!" // [Expect] d is Falsy!
