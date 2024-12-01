// No items
print [].map(fn(x) -> log(x)) // [expect] []

// Passing an anonymous function
print [1,3,4].map(fn(x) -> x + 1) // [expect] [2, 4, 5]

let a = []
print [1,3,4].map(fn(x) -> a.push(x))  // [expect] [nil, nil, nil]
print a                                // [expect] [1, 3, 4]

// Passing an anonymous function with two arguments
print [1,3,4].map(fn(x, i) -> x + i) // [expect] [1, 4, 6]

// Passing a named function
fn square(x) -> x * x
print [1,3,4].map(square) // [expect] [1, 9, 16]

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) ret x + 1 }
}
let storage = Storage()
print [1,3,4].map(storage.store)  // [expect] [2, 4, 5]
print storage.cache               // [expect] [1, 3, 4]
print [5,6,8].map(storage.store)  // [expect] [6, 7, 9]
print storage.cache               // [expect] [1, 3, 4, 5, 6, 8]

// Fuzzy test
print [1,2,3].map(fn (x) -> x * 2)             // [expect] [2, 4, 6]
print [1,2,3].map(fn (x) { })                  // [expect] [nil, nil, nil]
print ["a","b","c"].map(fn (x, i) -> ({i: x})) // [expect] [{0: a}, {1: b}, {2: c}]

// Side effects
let b = [1,2,3]
print a.map(fn(x) {
  b = [4,5,6]
  ret x == 6
}) // [expect] [false, false, false]
print b // [expect] [4, 5, 6]

import Gc
Gc.stress(true) // This is set to true by default in the test runner - just to be explicit

// Fuzzy test with GC
let c = [1,2,3]
print c.map(fn (x) {
  let trigger_gc = {1: "a", 2: "b", 3: "c"} 
  Gc.collect() // Will happen anyway, since the GC is stressed
  ret trigger_gc[x]
}) // [expect] [a, b, c]

