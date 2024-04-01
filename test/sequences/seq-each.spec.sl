// Passing an anonymous function
print [1,3,4].each(fn(x) -> log(x)) 
// [Expect] 1
// [Expect] 3
// [Expect] 4
// [Expect] nil

let a = []
print [1,3,4].each(fn(x) -> a.push(x)) // [Expect] nil
print a                                // [Expect] [1, 3, 4]

// Passing an anonymous function with two arguments
print [1,3,4].each(fn(x, i) -> log(x, i))
// [Expect] 1 0
// [Expect] 3 1
// [Expect] 4 2
// [Expect] nil

// Passing a named function
let s = []
fn store(x) -> s.push(x*2)
print [1,3,4].each(store) // [Expect] nil
print s              // [Expect] [2, 6, 8]

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) }
}
let storage = Storage()
print [1,3,4].each(storage.store) // [Expect] nil
print storage.cache               // [Expect] [1, 3, 4]
print [5,6,8].each(storage.store) // [Expect] nil
print storage.cache               // [Expect] [1, 3, 4, 5, 6, 8]