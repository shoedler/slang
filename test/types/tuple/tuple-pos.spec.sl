// No items
print (,).pos(fn(x) -> x) // [expect] nil

// Passing a value
print (1,3,4).pos(3) // [expect] 1

// Passing a value that does not exist
print (1,3,4).pos(2) // [expect] nil

// Passing an anonymous function
print (1,3,4).pos(fn(x) -> x == 3) // [expect] 1
print (1,3,4).pos(fn(x) -> x/2 == 2) // [expect] 2

// Returns the index of the first element that satisfies the condition
print (1,3,4).pos(fn(x) -> typeof(x) == Int) // [expect] 0
// ... or nil if no element satisfies the condition
print [true, false].pos(fn(x) -> x is Num) // [expect] nil

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print (1,3,4).pos(is_num) // [expect] 0

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print (1,3,4).pos(equals.num_3) // [expect] 1
equals.num = 4
print (1,3,4).pos(equals.num_3) // [expect] 2

// Passing a function that does not return a boolean
print (1,2,3).pos(fn(x) -> x) // [expect] nil

// Side effects
let a = (1,2,3)
print a.pos(fn (x) {
  a = (9,9,9)
  ret x == 6
}) // [expect] nil
print a // [expect] (9, 9, 9)
