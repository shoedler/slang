// No arguments
print try {}.has() else error // [expect] Expected 1 argument but got 0.

// No items
print {}.has(1) // [expect] false

// Passing a non-callable value
print {"a": 1}.has("b") // [expect] false
print {"a": 1}.has("a") // [expect] true

// Passing an anonymous function
print {"a": 1}.has(fn(x) -> x == "a") // [expect] true
print {"a": 1}.has(fn(x) -> x == "b") // [expect] false

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print {3: 1}.has(is_num) // [expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print {3: 1}.has(equals.num_3) // [expect] true
equals.num = 2
print {3: 1}.has(equals.num_3) // [expect] false

// Passing a function that does not return a boolean
print {"a": 1}.has(fn(x) -> x) // [expect] false

// Side effects
let a = {1:1}
print a.has(fn(x) {
  a = {2:2}
  ret x == 2
}) // [expect] false
print a // [expect] {2: 2}