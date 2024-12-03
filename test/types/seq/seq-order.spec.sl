
// Works for floats
let asc = fn (a, b) -> a < b ? -1 : a > b ? 1 : 0
let des = fn (a, b) -> b < a ? -1 : b > a ? 1 : 0
print [0.11,0.12,0.1,-0.11,-0.1,-0.12].order(asc) // [expect] [-0.12, -0.11, -0.1, 0.1, 0.11, 0.12]
print [0.11,0.12,0.1,-0.11,-0.1,-0.12].order(des) // [expect] [0.12, 0.11, 0.1, -0.1, -0.11, -0.12]
print [0.1,0.1,0.1,0.2,0.1,0.1].order(asc) // [expect] [0.1, 0.1, 0.1, 0.1, 0.1, 0.2]
print [0.1,0.1,0.1,0.2,0.1,0.1].order(des) // [expect] [0.2, 0.1, 0.1, 0.1, 0.1, 0.1]

// Works for ints
asc = fn (a,b) -> a-b
des = fn (a,b) -> b-a
print [44,3,-123, 23, 99, -0].order(asc) // [expect] [-123, 0, 3, 23, 44, 99]
print [44,3,-123, 23, 99, -0].order(des) // [expect] [99, 44, 23, 3, 0, -123]
print [2,2,2,2,0,0,1].order(asc) // [expect] [0, 0, 1, 2, 2, 2, 2]
print [2,2,2,2,0,0,1].order(des) // [expect] [2, 2, 2, 2, 1, 0, 0]

// ...works for anything, if the function is correct
const str_len = fn (a, b) -> a.len - b.len
print ["aa", "aaa", "a", ""].order(str_len) // [expect] [, a, aa, aaa]

// ... as long as the provided function is correct (i.e. doesn't return an int)
print try [1, 0].order(fn (a, b) -> nil) else error // [expect] Comparison Fn must return an Int. Got Nil.

// ... or, if the arity is wrong
print try [1, 0].order(fn (a) -> 1) else error // [expect] Function passed to "order" must take 2 arguments, but got 1.

// Works if the array is empty
print [].order(asc) // [expect] []

// Works for large arrays
const a = []
for let i = 100000; i > 0; i--; {
  a.push(i)
}
const b = a.order(asc)
print b[0] // [expect] 1
print b[b.len - 1] // [expect] 100000

// Passing a bound method
cls Storage {
  ctor { this.calls = 0 }
  fn tracking_asc(x, y) { this.calls++ ret this.asc(x, y) }
  fn asc(a, b) -> a - b
}
let storage = Storage()
print [10,1,3,4,5,6,7,8,2,9].order(storage.tracking_asc) // [expect] [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
print storage.calls // [expect] 23

// Doesn't sort in-place
const c = [1,2,3]
const d = c.order(asc)
print c // [expect] [1, 2, 3]
print d // [expect] [1, 2, 3]
print c == d // [expect] false

// Side effects 
const e = [1,2,3]
print e.order(fn(x, y) { e.push(x) ret y-x}) // [expect] [3, 2, 1]

import Gc
Gc.stress(true) // This is set to true by default in the test runner - just to be explicit

// Fuzzy test with GC
let f = [3,1,2]
print f.order(fn (x, y) {
  let trigger_gc = {"a": 1} 
  let k = trigger_gc["a"]
  Gc.collect() // Will happen anyway, since the GC is stressed
  ret x - k
}) // [expect] [1, 2, 3]
