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

// Fails if you declare more variables than there are elements
try { let [c, d] = [1]} catch { print error } // [Expect] Index out of bounds. Was 1, but this Seq has length 1.

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