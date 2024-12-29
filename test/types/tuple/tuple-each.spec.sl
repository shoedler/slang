// No items
print (,).each(fn (x) -> log(x)) // [expect] nil

// Passing an anonymous function
print (1,3,4).each(fn (x) -> log(x))
// [expect] 1
// [expect] 3
// [expect] 4
// [expect] nil

let a = (,)
print (1,3,4).each(fn(x) -> a = (9,9,9)) // [expect] nil
print a // [expect] (9, 9, 9)

// Passing an anonymous function with two arguments
print (1,3,4).each(fn(x, i) -> log(x, i)) 
// [expect] 1 0
// [expect] 3 1
// [expect] 4 2
// [expect] nil

// Passing an anonymous function without arguments
print (1,3,4).each(fn -> log("hi"))
// [expect] hi
// [expect] hi
// [expect] hi
// [expect] nil

// Passing a named function
fn my_log(x) { print x }
print (1,3,4).each(my_log) 
// [expect] 1
// [expect] 3
// [expect] 4
// [expect] nil

// Passing a bound method
cls Storage { 
  ctor { this.cache = [] }
  fn store(x) { this.cache.push(x) }
}
let storage = Storage()
print (1,3,4).each(storage.store) // [expect] nil
print storage.cache               // [expect] [1, 3, 4]
print (5,6,8).each(storage.store) // [expect] nil
print storage.cache               // [expect] [1, 3, 4, 5, 6, 8]

// Side effects
let b = [1,3,4]
b.each(fn(x) -> b.push(x))
print b                          // [expect] [1, 3, 4, 1, 3, 4]