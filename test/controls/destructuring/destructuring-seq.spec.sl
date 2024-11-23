// Works in global scope
let [a, b] = [1, 2, 3] 
print a // [expect] 1
print b // [expect] 2

// And in local scope
try {
  let [c, d] = [1,2]
  print c // [expect] 1
  print d // [expect] 2
} 

// Destructuring with only one element assigns nil to the variables without a value
let [e, f, g] = [1]
print e // [expect] 1
print f // [expect] nil
print g // [expect] nil

// Rest parameters are allowed on the end
let [h, ...i] = [1, 2, 3]
print h // [expect] 1
print i // [expect] [2, 3]

// Rest must contain can be empty
let [j, ...k] = [1]
print j // [expect] 1
print k // [expect] []

// Also works for strings
let [l, m] = "ab" 
print l // [expect] a
print m // [expect] b

// ...including rest parameters
let [n, ...o] = "abc"
print n // [expect] a
print o // [expect] bc

// Same rules apply for strings
let [p, q] = "a"
print p // [expect] a
print q // [expect] nil

let [r, ...s] = "a"
print r // [expect] a
print "\"" + s + "\"" // [expect] ""

// Destructuring tuples into seq:
let [t, u] = (1, 2)
print t // [expect] 1
print u // [expect] 2

// ...is not fully supported yet. The rest will be returned as the same type as the rhs:
let [v, ...w] = (1, 2, 3)
print v // [expect] 1
print w // [expect] (2, 3)