// No items
print [].filter(fn(x) { }) // [expect] []

// Passing an anonymous function
print [1,3,4].filter(fn(x) -> x > 1) // [expect] [3, 4]

let a = []
print [1,3,4].filter(fn(x) -> a.push(x))  // [expect] []
print a                                   // [expect] [1, 3, 4]

// Passing an anonymous function with two arguments
print [1,3,4].filter(fn(x, i) -> (x + i) % 2 == 0) // [expect] [3, 4]

// Passing a named function
fn is_even(x) -> x % 2 == 0
print [1,3,4].filter(is_even) // [expect] [4]

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) ret this.is_even(x) }
  fn is_even(x) -> x % 2 == 0
}
let storage = Storage()
print [1,3,4].filter(storage.store)  // [expect] [4]
print storage.cache                  // [expect] [1, 3, 4]
print [5,6,8].filter(storage.store)  // [expect] [6, 8]
print storage.cache                  // [expect] [1, 3, 4, 5, 6, 8]

// Side effects
let b = [1,2,3]
print b.filter(fn(x) { b.push(x) }) // [expect] []
print b                             // [expect] [1, 2, 3, 1, 2, 3]

// Fuzzy test to try to trigger the GC
let c = [1,2,3]
print c.filter(fn (x) {
  let trigger_gc = {"a": 2} // by creating a new object
  let k = trigger_gc["a"]
  ret x == k
}) // [expect] [2]
