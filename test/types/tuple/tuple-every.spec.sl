// No items
print (,).every(fn(x) -> x == x) // [Expect] true

// Passing an anonymous function
print (1,3,4).every(fn(x) -> x == 3) // [Expect] false
print (1,3,4).every(fn(x) -> x/2 == 2) // [Expect] false
print (1,3,4).every(fn(x) -> typeof(x) == Int) // [Expect] true

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print (1,3,4).every(is_num) // [Expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print (1,3,4).every(equals.num_3) // [Expect] false
equals.num = 4
print (1,3,4).every(equals.num_3) // [Expect] false

// Passing a function that does not return a boolean
print (1,2,3).every(fn(x) -> x) // [Expect] false

// Fuzzy tests
print (1,2,3).every(fn(x) -> x != 4) // [Expect] true
print (,).every(fn (x) -> x > 0) // [Expect] true
print (1,2,3).every(fn (x) -> x > 1) // [Expect] false
print (1,2,3).every(fn (x) -> x > 0) // [Expect] true
print (1,2,3).every(fn (x) -> x > 0 and x < 3) // [Expect] false

// Side effects
let a = (1,2,3)
print a.every(fn(x) {
  a = (9,9,9)
  ret x < 3
}) // [Expect] false
print a // [Expect] (9, 9, 9)