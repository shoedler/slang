// No items
print (,).fold(1, fn(acc, x) -> acc + x) // [expect] 1

// Passing an anonymous function
print (1,3,4).fold(0, fn(acc, x) -> acc + x) // [expect] 8
print ["1","3","4"].fold("0", fn(acc, x) -> acc + x) // [expect] 0134

// Passing an anonymous function with index
print (1,3,4).fold(0, fn(acc, x, i) -> acc + x + i) // [expect] 11

// Passing a named function
fn sum(a, b) -> a + b
print (1,3,4).fold(0, sum) // [expect] 8

// Passing a bound method
cls Calc { 
  ctor { this.num = 2 }
  fn mult(a, b) -> a * b * this.num
}
let calc = Calc()
print (1,3,4).fold(1, calc.mult) // [expect] 96
calc.num = 3
print (1,3,4).fold(1, calc.mult) // [expect] 324

// Fuzzy tests
print (1,2,3).fold(0, fn(acc,x) -> acc + x) // [expect] 6
print {1:10, true:10, nil:10, []:10, "9": "10"}
  .values()
  .fold("", fn (acc, x) -> acc + x.to_str()) // [expect] 1010101010

// Side effects
let a = (1,2,3)
print a.fold("", fn(acc, x) -> (a = (9,9,9)).to_str() + acc) // [expect] (9, 9, 9)(9, 9, 9)(9, 9, 9)
print a // [expect] (9, 9, 9)