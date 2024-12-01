// No items
print [].sift(fn(x) { }) // [expect] []

// Passing an anonymous function
print [1,3,4].sift(fn(x) -> x > 1) // [expect] [3, 4]

let a = []
print [1,3,4].sift(fn(x) -> a.push(x)) // [expect] []
print a                                // [expect] [1, 3, 4]

// Passing an anonymous function with two arguments
print [1,3,4].sift(fn(x, i) -> (x + i) % 2 == 0) // [expect] [3, 4]

// Passing a named function
fn is_even(x) -> x % 2 == 0
print [1,3,4].sift(is_even) // [expect] [4]

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) ret this.is_even(x) }
  fn is_even(x) -> x % 2 == 0
}
let storage = Storage()
print [1,3,4].sift(storage.store) // [expect] [4]
print storage.cache               // [expect] [1, 3, 4]
print [5,6,8].sift(storage.store) // [expect] [6, 8]
print storage.cache               // [expect] [1, 3, 4, 5, 6, 8]

// Side effects
let b = [1,2,3]
print b.sift(fn(x) { b.push(x) }) // [expect] []
print b                            // [expect] [1, 2, 3, 1, 2, 3]

import Gc
Gc.stress(true) // This is set to true by default in the test runner - just to be explicit

// Fuzzy test with GC
let c = [1,2,3]
print c.sift(fn (x) {
  let trigger_gc = {"a": 2} 
  let k = trigger_gc["a"]
  Gc.collect() // Will happen anyway, since the GC is stressed
  ret x == k
}) // [expect] [2]
