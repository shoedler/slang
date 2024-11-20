// No items
print [].last(fn(x) -> x) // [Expect] nil

// Passing an anonymous function
print [1,3,4].last(fn(x) -> x == 3) // [Expect] 3
print [1,3,4].last(fn(x) -> x/2 == 2) // [Expect] 4

// Actually returns the last element that satisfies the condition
print [1,3,4].last(fn(x) -> typeof(x) == Int) // [Expect] 4

// Passing a named function
fn is_num(x) {
  ret x is Num
}
print [1,3,4].last(is_num) // [Expect] 4

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num }
}
let equals = Equals()
print [1,3,4].last(equals.num_3) // [Expect] 3
equals.num = 4
print [1,3,4].last(equals.num_3) // [Expect] 4

// Passing a function that does not return a boolean
print [1,2,3].last(fn(x) -> x) // [Expect] nil

// Side effects
let a = [1,2,3]
print a.last(fn(x) {
  a = [4,5,6]
  ret x == 6
}) // [Expect] nil
print a // [Expect] [4, 5, 6]
