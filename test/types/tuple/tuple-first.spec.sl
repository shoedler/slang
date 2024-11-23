// No items
print (,).first(fn(x) -> x) // [expect] nil

// Passing an anonymous function
print (1,3,4).first(fn(x) -> x == 3) // [expect] 3
print (1,3,4).first(fn(x) -> x/2 == 2) // [expect] 4

// Actually returns the first element that satisfies the condition
print (1,3,4).first(fn(x) -> typeof(x) == Int) // [expect] 1

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print (1,3,4).first(is_num) // [expect] 1

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print (1,3,4).first(equals.num_3) // [expect] 3
equals.num = 4
print (1,3,4).first(equals.num_3) // [expect] 4

// Passing a function that does not return a boolean
print (1,2,3).first(fn(x) -> x) // [expect] nil

// Side effects
let a = (1,2,3)
print a.first(fn(x) {
  a = (9,9,9)
  ret x == 6
}) // [expect] nil
print a // [expect] (9, 9, 9)
