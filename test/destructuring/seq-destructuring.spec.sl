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

// Rest must contain at least one element
try { let [g, ...h] = [1] } catch { print error } // [Expect] Slice indices out of bounds. Start resolved to 1 and end to 1, but this Seq has length 1.

// Also works for strings
let [i, j] = "ab" 
print i // [Expect] a
print j // [Expect] b

// ...including rest parameters
let [k, ...l] = "abc"
print k // [Expect] a
print l // [Expect] bc

// Same rules apply for strings
try { let [m, n] = "a" } catch { print error } // [Expect] Index out of bounds. Was 1, but this Str has length 1.
try { let [m, ...n] = "a" } catch { print error } // [Expect] Slice indices out of bounds. Start resolved to 1 and end to 1, but this Str has length 1.