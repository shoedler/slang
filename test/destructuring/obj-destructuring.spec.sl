// Works in global scope
let {a, b} = {"a": 1, "b": 2, "c": 3}
print a // [Expect] 1
print b // [Expect] 2

// And in local scope
try {
  let {c, d} = {"c": 1, "d": 2}
  print c // [Expect] 1
  print d // [Expect] 2
} 

// Destructuring with more variables than values assigns nil to the variables without a value
let {e, f, g} = {"e": 1, "f": 2}
print e // [Expect] 1
print f // [Expect] 2
print g // [Expect] nil
