// No items
print (,).some(fn(x) -> x == x) // [Expect] false

// Passing an anonymous function
print (1,3,4).some(fn(x) -> x == 3) // [Expect] true
print (1,3,4).some(fn(x) -> x/2 == 2) // [Expect] true
print (1,3,4).some(fn(x) -> typeof(x) == Int) // [Expect] true

// Passing a named function
fn is_num(x) {
  ret x is Num;
}
print (1,3,4).some(is_num) // [Expect] true

// Passing a bound method
cls Equals { 
  ctor { this.num = 3 }
  fn num_3(x) { ret x == this.num; }
}
let equals = Equals()
print (1,3,4).some(equals.num_3) // [Expect] true
equals.num = 4
print (1,3,4).some(equals.num_3) // [Expect] true

// Passing a function that does not return a boolean
print (1,2,3).some(fn(x) -> x) // [Expect] false

// Fuzzy tests
print (1,2,3).some(fn(x) -> x == 4) // [Expect] false
print (,).some(fn (x) -> x > 0) // [Expect] false
print (1,2,3).some(fn (x) -> x > 1) // [Expect] true
print (1,2,3).some(fn (x) -> x > 3) // [Expect] false
print (1,2,3).some(fn (x) -> x > 1 and x < 3) // [Expect] true