// No items
print [].every(fn(x) -> x == x) // [Expect] true

// Passing an anonymous function
print [1,3,4].every(fn(x) -> x == 3) // [Expect] false
print [1,3,4].every(fn(x) -> x/2 == 2) // [Expect] false
print [1,3,4].every(fn(x) -> type_of(x) == Num) // [Expect] true

// Passing a named function
fn is_num(x) {
  ret type_of(x) == Num;
}
print [1,3,4].every(is_num) // [Expect] true

// Passing a bound method
cls Is { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let is = Is()
print [1,3,4].every(is.num_3) // [Expect] false
is.num = 4
print [1,3,4].every(is.num_3) // [Expect] false

// Passing a function that does not return a boolean
print [1,2,3].every(fn(x) -> x) // [Expect] false

// Fuzzy tests
print [1,2,3].every(fn(x) -> x != 4) // [Expect] true