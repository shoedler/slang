// Passing an anonymous function
print [1,3,4].first(fn(x) -> x == 3) // [Expect] 3
print [1,3,4].first(fn(x) -> x/2 == 2) // [Expect] 4

// Actually returns the first element that satisfies the condition
print [1,3,4].first(fn(x) -> type_of(x) == Num) // [Expect] 1

// Passing a named function
fn is_num(x) {
  ret type_of(x) == Num;
}
print [1,3,4].first(is_num) // [Expect] 1

// Passing a bound method
cls Is { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let is = Is()
print [1,3,4].first(is.num_3) // [Expect] 3
is.num = 4
print [1,3,4].first(is.num_3) // [Expect] 4

// Passing a function that does not return a boolean
print [1,2,3].first(fn(x) -> x) // [Expect] nil