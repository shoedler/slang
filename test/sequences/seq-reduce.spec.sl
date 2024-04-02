// No items
print [].reduce(1, fn(acc, x) -> acc + x) // [Expect] 1

// Passing an anonymous function
print [1,3,4].reduce(0, fn(acc, x) -> acc + x) // [Expect] 8
print ["1","3","4"].reduce("0", fn(acc, x) -> acc + x) // [Expect] 0134

// Passing an anonymous function with index
print [1,3,4].reduce(0, fn(acc, x, i) -> acc + x + i) // [Expect] 11

// Passing a named function
fn sum(a, b) -> a + b
print [1,3,4].reduce(0, sum) // [Expect] 8

// Passing a bound method
cls Calc { 
  ctor { this.num = 2 }
  fn mult(a, b) -> a * b * this.num
}
let calc = Calc()
print [1,3,4].reduce(1, calc.mult) // [Expect] 96
calc.num = 3
print [1,3,4].reduce(1, calc.mult) // [Expect] 324

// Fuzzy tests
print [1,2,3].reduce(0, fn(acc,x) -> acc + x) // [Expect] 6
print {1:10, true:10, nil:10, []:10, "9": "10"}
  .values()
  .reduce("", fn (acc, x) -> acc + x.to_str()) // [Expect] 1010101010