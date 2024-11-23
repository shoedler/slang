// No items
print (,).every(fn(x) -> x == x) // [expect] true

// Passing an anonymous function
print (1,3,4).every(fn(x) -> x == 3) // [expect] false
print (1,3,4).every(fn(x) -> x/2 == 2) // [expect] false
print (1,3,4).every(fn(x) -> typeof(x) == Int) // [expect] true

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print (1,3,4).every(is_num) // [expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print (1,3,4).every(equals.num_3) // [expect] false
equals.num = 4
print (1,3,4).every(equals.num_3) // [expect] false

// Passing a function that does not return a boolean
print (1,2,3).every(fn(x) -> x) // [expect] false

// Fuzzy tests
print (1,2,3).every(fn(x) -> x != 4) // [expect] true
print (,).every(fn (x) -> x > 0) // [expect] true
print (1,2,3).every(fn (x) -> x > 1) // [expect] false
print (1,2,3).every(fn (x) -> x > 0) // [expect] true
print (1,2,3).every(fn (x) -> x > 0 and x < 3) // [expect] false

// Side effects
let a = (1,2,3)
print a.every(fn(x) {
  a = (9,9,9)
  ret x < 3
}) // [expect] false
print a // [expect] (9, 9, 9)