// Works in global scope
let {a, b} = {"a": 1, "b": 2, "c": 3}
print a // [Expect] 1
print b // [Expect] 2

// And in local scope
try {
  let {a, b} = {"a": 1, "b": 2}
  print a // [Expect] 1
  print b // [Expect] 2
} 

// Destructuring with more variables than values assigns nil to the variables without a value
let {a, b, c} = {"a": 1, "b": 2}
print a // [Expect] 1
print b // [Expect] 2
print c // [Expect] nil