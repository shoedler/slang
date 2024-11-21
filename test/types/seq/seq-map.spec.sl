// No items
print [].map(fn(x) -> log(x)) // [Expect] []

// Passing an anonymous function
print [1,3,4].map(fn(x) -> x + 1) // [Expect] [2, 4, 5]

let a = []
print [1,3,4].map(fn(x) -> a.push(x))  // [Expect] [nil, nil, nil]
print a                                // [Expect] [1, 3, 4]

// Passing an anonymous function with two arguments
print [1,3,4].map(fn(x, i) -> x + i) // [Expect] [1, 4, 6]

// Passing a named function
fn square(x) -> x * x
print [1,3,4].map(square) // [Expect] [1, 9, 16]

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) ret x + 1 }
}
let storage = Storage()
print [1,3,4].map(storage.store)  // [Expect] [2, 4, 5]
print storage.cache               // [Expect] [1, 3, 4]
print [5,6,8].map(storage.store)  // [Expect] [6, 7, 9]
print storage.cache               // [Expect] [1, 3, 4, 5, 6, 8]

// Fuzzy test
print [1,2,3].map(fn (x) -> x * 2)             // [Expect] [2, 4, 6]
print [1,2,3].map(fn (x) { })                  // [Expect] [nil, nil, nil]
print ["a","b","c"].map(fn (x, i) -> ({i: x})) // [Expect] [{0: a}, {1: b}, {2: c}]

// Side effects
let b = [1,2,3]
print a.map(fn(x) {
  b = [4,5,6]
  ret x == 6
}) // [Expect] [false, false, false]
print b // [Expect] [4, 5, 6]

import Gc
Gc.stress(true) // This is set to true by default in the test runner - just to be explicit

// Fuzzy test to try to trigger the GC
let c = [1,2,3]
print c.map(fn (x) {
  let trigger_gc = {1: "a", 2: "b", 3: "c"} // by creating a new object
  ret trigger_gc[x]
}) // [Expect] [a, b, c]

