// Works in global scope
let [a, b] = [1, 2, 3] 
print a // [Expect] 1
print b // [Expect] 2

// And in local scope
try {
  let [a, b] = [1,2]
  print a // [Expect] 1
  print b // [Expect] 2
} 

// Destructuring with only one element assigns nil to the variables without a value
let [a, b, c] = [1]
print a // [Expect] 1
print b // [Expect] nil
print c // [Expect] nil

// Rest parameters are allowed on the end
let [e, ...f] = [1, 2, 3]
print e // [Expect] 1
print f // [Expect] [2, 3]

// Rest must contain can be empty
let [g, ...h] = [1]
print g // [Expect] 1
print h // [Expect] []

// Also works for strings
let [i, j] = "ab" 
print i // [Expect] a
print j // [Expect] b

// ...including rest parameters
let [k, ...l] = "abc"
print k // [Expect] a
print l // [Expect] bc

// Same rules apply for strings
let [m, n] = "a"
print m // [Expect] a
print n // [Expect] nil

let [o, ...p] = "a"
print o // [Expect] a
print "\"" + p + "\"" // [Expect] ""

// Destructuring tuples into seq:
let [q, r] = (1, 2)
print q // [Expect] 1
print r // [Expect] 2

// ...is not fully supported yet. The rest will be returned as the same type as the rhs:
let [s, ...t] = (1, 2, 3)
print s // [Expect] 1
print t // [Expect] (2, 3)