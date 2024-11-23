// No items
print [].each(fn(x) -> log(x)) // [expect] nil

// Passing an anonymous function
print [1,3,4].each(fn(x) -> log(x)) 
// [expect] 1
// [expect] 3
// [expect] 4
// [expect] nil

let a = []
print [1,3,4].each(fn(x) -> a.push(x)) // [expect] nil
print a                                // [expect] [1, 3, 4]

// Passing an anonymous function with two arguments
print [1,3,4].each(fn(x, i) -> log(x, i))
// [expect] 1 0
// [expect] 3 1
// [expect] 4 2
// [expect] nil

// Passing a named function
let s = []
fn store(x) -> s.push(x*2)
print [1,3,4].each(store) // [expect] nil
print s              // [expect] [2, 6, 8]

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) }
}
let storage = Storage()
print [1,3,4].each(storage.store) // [expect] nil
print storage.cache               // [expect] [1, 3, 4]
print [5,6,8].each(storage.store) // [expect] nil
print storage.cache               // [expect] [1, 3, 4, 5, 6, 8]

// Side effects
let b = [1,3,4]
print b.each(fn(x) -> b.push(x)) // [expect] nil
print b                          // [expect] [1, 3, 4, 1, 3, 4]