// No arguments
print try {}.has() else error // [Expect] Expected 1 arguments but got 0.

// No items
print {}.has(1) // [Expect] false

// Passing a non-callable value
print {"a": 1}.has("b") // [Expect] false
print {"a": 1}.has("a") // [Expect] true

// Passing an anonymous function
print {"a": 1}.has(fn(x) -> x == "a") // [Expect] true
print {"a": 1}.has(fn(x) -> x == "b") // [Expect] false

// Passing a named function
fn is_num(x) {
  ret typeof(x) == Num;
}
print {3: 1}.has(is_num) // [Expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let equals = Equals()
print {3: 1}.has(equals.num_3) // [Expect] true
equals.num = 2
print {3: 1}.has(equals.num_3) // [Expect] false

// Passing a function that does not return a boolean
print {"a": 1}.has(fn(x) -> x) // [Expect] false

// Side effects
let a = {1:1}
print a.has(fn(x) {
  a = {2:2}
  ret x == 2;
}) // [Expect] false
print a // [Expect] {2: 2}