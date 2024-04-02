// No items
print [].count(fn(x) -> x > 0) // [Expect] 0

// Passing a value
print [1,3,4].count(1) // [Expect] 1

// Passing an anonymous function
print [1,3,4].count(fn(x) -> x > 1) // [Expect] 2

// Passing a named function
fn is_num(x) {
  ret type_of(x) == Num;
}
print [1,3,4].count(is_num) // [Expect] 3

// Passing a bound method
cls Is { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let is = Is()
print [1,3,4,1].count(is.num_3) // [Expect] 1
is.num = 1
print [1,3,4,1].count(is.num_3) // [Expect] 2

// Passing a function that does not return a boolean
print [1,2,3].count(fn(x) -> x) // [Expect] 0

// Fuzzy tests
print [1,2,3].count(fn(x) -> x != 4) // [Expect] 3
print [1,2,3].count(fn (x) -> x > 1) // [Expect] 2
print [1,2,3].count(fn (x) -> x > 0) // [Expect] 3
print [1,2,3].count(fn (x) -> x > 0 and x < 3) // [Expect] 2

// Side effects
let a = [1,2,3]
print a.count(fn(x) {
  a.push(4)
  ret x > 1;
}) // [Expect] 2
print a // [Expect] [1, 2, 3, 4, 4, 4]