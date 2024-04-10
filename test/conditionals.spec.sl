let a = 123
if a a = 321
print a // [Expect] 321

let b = "Hi"
if !b b = "?" else b = b + " Wrld"
print b // [Expect] Hi Wrld

let c

// 'and' and 'or'
if a and b print "a and b is Truthy!"                                // [Expect] a and b is Truthy!
if a or b print "a or b is Truthy!"                                  // [Expect] a or b is Truthy!
if a and c print "a and c is Truthy!" else print "a and c is Falsy!" // [Expect] a and c is Falsy!

// Ternary operator
print a ? "a is Truthy!" : "a is Falsy!" // [Expect] a is Truthy!
print b ? "b is Truthy!" : "b is Falsy!" // [Expect] b is Truthy!
print c ? "c is Truthy!" : "c is Falsy!" // [Expect] c is Falsy!