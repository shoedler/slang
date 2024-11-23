// No arguments
print try [1,2,3].has() else error // [expect] Expected 1 argument but got 0.

// No items
print [].has(1) // [expect] false

// Passing a non-callable value
print [1,3,4].has(2) // [expect] false
print [1,3,4].has(3) // [expect] true

// Passing an anonymous function
print [1,3,4].has(fn(x) -> x == 3) // [expect] true
print [1,3,4].has(fn(x) -> x == 2) // [expect] false

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print [1,3,4].has(is_num) // [expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print [1,3,4].has(equals.num_3) // [expect] true
equals.num = 2
print [1,3,4].has(equals.num_3) // [expect] false

// Passing a function that does not return a boolean
print [1,2,3].has(fn(x) -> x) // [expect] false

// Side effects
let a = [1,2,3]
print a.has(fn(x) {
  a = [4,5,6]
  ret x == 6
}) // [expect] false
print a // [expect] [4, 5, 6]