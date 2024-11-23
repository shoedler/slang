// No items
print (,).last(fn(x) -> x) // [expect] nil

// Passing an anonymous function
print (1,3,4).last(fn(x) -> x == 3) // [expect] 3
print (1,3,4).last(fn(x) -> x/2 == 2) // [expect] 4

// Actually returns the last element that satisfies the condition
print (1,3,4).last(fn(x) -> typeof(x) == Int) // [expect] 4

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print (1,3,4).last(is_num) // [expect] 4

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print (1,3,4).last(equals.num_3) // [expect] 3
equals.num = 4
print (1,3,4).last(equals.num_3) // [expect] 4

// Passing a function that does not return a boolean
print (1,2,3).last(fn(x) -> x) // [expect] nil

// Side effects
let a = (1,2,3)
print a.last(fn(x) {
  a = (9,9,9)
  ret x == 6
}) // [expect] nil
print a // [expect] (9, 9, 9)
