// Works in global scope
let [a, b] = [1, 2, 3] 
print a // [Expect] 1
print b // [Expect] 2

// And in local scope
try {
  let [c, d] = [1,2]
  print c // [Expect] 1
  print d // [Expect] 2
} 

// Destructuring with only one element assigns nil to the variables without a value
let [e, f, g] = [1]
print e // [Expect] 1
print f // [Expect] nil
print g // [Expect] nil

// Rest parameters are allowed on the end
let [h, ...i] = [1, 2, 3]
print h // [Expect] 1
print i // [Expect] [2, 3]

// Rest must contain can be empty
let [j, ...k] = [1]
print j // [Expect] 1
print k // [Expect] []

// Also works for strings
let [l, m] = "ab" 
print l // [Expect] a
print m // [Expect] b

// ...including rest parameters
let [n, ...o] = "abc"
print n // [Expect] a
print o // [Expect] bc

// Same rules apply for strings
let [p, q] = "a"
print p // [Expect] a
print q // [Expect] nil

let [r, ...s] = "a"
print r // [Expect] a
print "\"" + s + "\"" // [Expect] ""

// Destructuring tuples into seq:
let [t, u] = (1, 2)
print t // [Expect] 1
print u // [Expect] 2

// ...is not fully supported yet. The rest will be returned as the same type as the rhs:
let [v, ...w] = (1, 2, 3)
print v // [Expect] 1
print w // [Expect] (2, 3)