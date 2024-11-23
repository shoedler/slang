// Works in global scope
let {a, b} = {"a": 1, "b": 2, "c": 3}
print a // [expect] 1
print b // [expect] 2

// And in local scope
try {
  let {c, d} = {"c": 1, "d": 2}
  print c // [expect] 1
  print d // [expect] 2
} 

// Destructuring with more variables than values assigns nil to the variables without a value
let {e, f, g} = {"e": 1, "f": 2}
print e // [expect] 1
print f // [expect] 2
print g // [expect] nil
