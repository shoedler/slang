// No items
print [].index_of(fn(x) -> x) // [Expect] nil

// Passing a value
print [1,3,4].index_of(3) // [Expect] 1

// Passing a value that does not exist
print [1,3,4].index_of(2) // [Expect] nil

// Passing an anonymous function
print [1,3,4].index_of(fn(x) -> x == 3) // [Expect] 1
print [1,3,4].index_of(fn(x) -> x/2 == 2) // [Expect] 2

// Returns the index of the first element that satisfies the condition
print [1,3,4].index_of(fn(x) -> typeof(x) == Num) // [Expect] 0
// ... or nil if no element satisfies the condition
print [true, false].index_of(fn(x) -> x is Num) // [Expect] nil

// Passing a named function
fn is_num(x) {
  ret typeof(x) == Num;
}
print [1,3,4].index_of(is_num) // [Expect] 0

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let equals = Equals()
print [1,3,4].index_of(equals.num_3) // [Expect] 1
equals.num = 4
print [1,3,4].index_of(equals.num_3) // [Expect] 2

// Passing a function that does not return a boolean
print [1,2,3].index_of(fn(x) -> x) // [Expect] nil

// Side effects
let a = [1,2,3]
print a.index_of(fn (x) {
  a = [4,5,6]
  ret x == 6;
}) // [Expect] nil
print a // [Expect] [4, 5, 6]
