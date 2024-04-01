// No items
print [].has(1) // [Expect] false

// Passing a non-callable value
print [1,3,4].has(2) // [Expect] false
print [1,3,4].has(3) // [Expect] true

// Passing an anonymous function
print [1,3,4].has(fn(x) -> x == 3) // [Expect] true
print [1,3,4].has(fn(x) -> x == 2) // [Expect] false

// Passing a named function
fn is_num(x) {
  ret type_of(x) == Num;
}
print [1,3,4].has(is_num) // [Expect] true

// Passing a bound method
cls Is { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let is = Is()
print [1,3,4].has(is.num_3) // [Expect] true
is.num = 2
print [1,3,4].has(is.num_3) // [Expect] false

// Passing a function that does not return a boolean
print [1,2,3].has(fn(x) -> x) // [Expect] false