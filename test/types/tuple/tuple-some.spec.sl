// No items
print (,).some(fn(x) -> x == x) // [expect] false

// Passing an anonymous function
print (1,3,4).some(fn(x) -> x == 3) // [expect] true
print (1,3,4).some(fn(x) -> x/2 == 2) // [expect] true
print (1,3,4).some(fn(x) -> typeof(x) == Int) // [expect] true

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print (1,3,4).some(is_num) // [expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print (1,3,4).some(equals.num_3) // [expect] true
equals.num = 4
print (1,3,4).some(equals.num_3) // [expect] true

// Passing a function that does not return a boolean
print (1,2,3).some(fn(x) -> x) // [expect] false

// Fuzzy tests
print (1,2,3).some(fn(x) -> x == 4) // [expect] false
print (,).some(fn (x) -> x > 0) // [expect] false
print (1,2,3).some(fn (x) -> x > 1) // [expect] true
print (1,2,3).some(fn (x) -> x > 3) // [expect] false
print (1,2,3).some(fn (x) -> x > 1 and x < 3) // [expect] true