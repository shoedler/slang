// Works in global scope
let [a, b] = [1, 2, 3] 
print a // [Expect] 1
print b // [Expect] 2

try {
  let [a, b] = [1,2]
  print a // [Expect] 1
  print b // [Expect] 2

  let [c, d] = [1] // Will fail
  print c // Won't run
  print d // Won't run
} catch { 
  print error // [Expect] Index out of bounds. Was 1, but this Seq has length 1.
}

let [e, ...f] = [1, 2, 3]
print e // [Expect] 1
print f // [Expect] [2, 3]