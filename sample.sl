// print "--------------------------------------------------------------------------------"
// print "Basic"
// print "--------------------------------------------------------------------------------"b
// print "string"
// print 123
// print 123.123
// print 0xDEADBEEF // 3735928559
// print 0b1010 // 10
// print 0o777 // 511

// print "--------------------------------------------------------------------------------"
// print "Conditionals"
// print "--------------------------------------------------------------------------------"
// // nil and false are falsy - everything else is considered truthy
// let a = 123
// if a a = 321
// print a // 321

// let b = "Hi"
// if !b b = "?" else b += " Wrld"
// print b // "Hi Wrld"

// // Short-circuiting operators: and, or
// if a and b print "a and b is Truthy!"
// if a or b print "a or b is Truthy!"

// // Ternary operators
// print {} ? "Truthy" : "(skip)" // "Truthy"
// print nil ? "(skip)" : false ? "(skip)" : true ? "Truthy" : "(skip)" // "Truthy"

// print "--------------------------------------------------------------------------------"
// print "Loops"
// print "--------------------------------------------------------------------------------"
// for let i = 0 ; i < 5 ; i++ ; {
//   print i
// }

// let b = false
// for b ; !b ;; { // Incrementing is optional
//   print b
//   b = true
// }

// let c = 0
// while b {
//   c = c + 1
//   b = c < 10
//   print c
// }

// for let i = 0 ; i < 100 ; i++ ; {
//   let outer = fn {
//     let x = "outside"
//     let inner = fn {
//       print x
//     }
//     ret inner
//   }

//   for let y = 0; y < 100; y = y + 1 ; {
//     let x = fn {
//       print "Hello World"
//     }
//     x()
//   }
//   let closure = outer()
//   closure()
//   print i
// }

// // skip (Continue) 
// for let i = 0; i < 10; i++; {
//   if i % 2 == 0
//     skip
//   print i.to_str() + " is odd"
// }

// // Break
// let i = 0
// while i++ <= 5 {
//   for let l = 0; l <= 5; l++; {
//     if l == 2 
//       break
//     print "l = " + l
//   }

//   if i % 2 == 0
//     break
// }

// print "--------------------------------------------------------------------------------"
// print "Functions"
// print "--------------------------------------------------------------------------------"
// let a = fn -> "a" // Anonymous Fn
// let a_args = fn(x,y,z) -> x + y + z // Anonymous Fn
// fn b -> "b" // Named Fn
// fn b_args(x,y,z) -> x + y + z // Named Fn

// print a
// print a_args
// print b
// print b_args

// print "--------------------------------------------------------------------------------"
// print "Functions can be recursive"
// print "--------------------------------------------------------------------------------"

// // With native functions. Here, timed with clock()
// let start = clock()
// print "Let's calculate fib(35)!"
// let result = fib(35)
// print clock() - start

// print result

// print "--------------------------------------------------------------------------------"
// print "Functions have / are Closures"
// print "--------------------------------------------------------------------------------"
// fn outer {
//   let x = "outside"
//   let inner = fn {
//     print x
//   }
//   inner()
// }

// outer()
// fn closure -> outer()
// closure()

// print "--------------------------------------------------------------------------------"
// print "Classes"
// print "--------------------------------------------------------------------------------"
// cls Scone {
//   fn topping(first, second) {
//     print "scone with " + first + " and " + second
//     ret 1
//   }
// }

// let scone = Scone()
// let res = scone.topping("berries", "cream") // Prints "scone with berries and cream"
// print res // Prints 1 
// print scone.topping // Prints [Fn topping, arity 2]

// cls Nested {
//   fn method {
//     let function = fn {
//       print this
//     }

//     function()
//   }
// }

// Nested().method() // Prints "Nested Instance"

// cls CoffeeMaker {
//   ctor (coffee) {
//     this.coffee = coffee
//   }

//   fn brew  {
//     print "Enjoy your cup of " + this.coffee

//     // No reusing the grounds!
//     this.coffee = nil
//   }
// }

// let maker = CoffeeMaker("coffee and chicory")
// maker.brew() 
// maker.brew() // Error

// print "--------------------------------------------------------------------------------"
// print "Inheritance"
// print "--------------------------------------------------------------------------------"
// cls A {
//   fn method {
//     print "A"
//   }
// }

// cls B : A {
//   fn method {
//     let closure = base.method
//     closure() // Prints "A"
//   }
// }

// let b = B()
// b.method()

// print "--------------------------------------------------------------------------------"
// print "Sequences"
// print "--------------------------------------------------------------------------------"
// let x = [1,2,3]
// print x
// print x[0]

// x[0] = 4
// print x[0]
// x[1] = ["Foo", "Bar"]
// print x

// cls List {
//   ctor {
//     this.list = [1,2,3]
//   }

//   fn printList {
//     print this.list
//   }
// }

// fn make_list -> [1,2,3]
// let s = List()
// s.printList()
// s.list[1] = -2
// s.printList()

// //
// // Seq.has
// //
// fn is_num(x) {
//   ret typeof(x) == Num
// }

// cls Validators { 
//   static fn is_num_3(x) { ret x == 3 }
// }

// print [1,3,4].has(2)
// print [1,3,4].has(3)
// print [1,3,4].has(is_num)
// print [1,3,4].has(Validators.is_num_3)
// Validators.num = 2
// print [1,3,4].has(Validators.is_num_3)

// // 
// // Seq.first
// //
// print [].first() // nil
// print [1,2,3].first() // 1
// print [1,2,3].first(fn (x) -> x/2 == 1) // 2

// //
// // Seq.last
// //
// print [].last() // nil
// print [1,2,3].last() // 3
// print [1,2,3].last(fn (x) -> typeof(x) == Num) // 3

// //
// // Seq.each
// //
// print [1,2,3].each(fn (x) -> log(x)) // [1, 2, 3]
// print [1,2,3].each(fn (x, i) -> log(x + i)) // [1, 3, 5]
// let a = [1,2,3]
// a.each(fn (x, i) {
//   a.push(x + 1)
// })
// print a // [1, 2, 3, 2, 3, 4]

// //
// // Seq.map
// //
// print [].map(fn (x) -> x * 2) // []
// print [1,2,3].map(fn (x) -> x * 2) // [2, 4, 6]
// print [1,2,3].map(fn (x) -> log(x)) // [nil, nil, nil]
// print ["a","b","c"].map(fn (x, i) -> ({i: x})) // [{0: "a"}, {1: "b"}, {2: "c"}]

// //
// // Seq.filter
// //
// print [].filter(fn (x) -> x > 0) // []
// print [1,2,3].filter(fn (x) -> x > 1) // [2, 3]
// print [1,2,3].filter(fn (x) {}) // []

// //
// // Seq.join
// //
// print [].join("") + "empty!" // "empty!"
// print [1,2,3].join("") // "1,2,3"
// print [1,2,3].join("") // "123"
// print [1,2,3].join(" ") // "1 2 3"
// print [nil, true, 1, "string", [1,2,3], {1 : 2}].join(",") // nil,true,1,string,[1, 2, 3],{1: 2}

// // 
// // Seq.reverse
// //
// print [].reverse() // []
// print [1,2,3].reverse() // [3, 2, 1]
// print [1,2,3].reverse().reverse() // [1, 2, 3]

// // 
// // Seq.some
// //
// print [].some(fn (x) -> x > 0) // false
// print [1,2,3].some(fn (x) -> x > 1) // true
// print [1,2,3].some(fn (x) -> x > 3) // false
// print [1,2,3].some(fn (x) -> x > 1 and x < 3) // true

// //
// // Seq.every
// //
// print [].every(fn (x) -> x > 0) // true
// print [1,2,3].every(fn (x) -> x > 1) // false
// print [1,2,3].every(fn (x) -> x > 0) // true
// print [1,2,3].every(fn (x) -> x > 0 and x < 3) // false

// // 
// // Seq.reduce
// //
// print [1,2,3].reduce(0, fn(acc,x) -> acc + x) // 6
// print {1:10, true:10, nil:10, []:10, "9": "10"}
//   .values()
//   .reduce("", fn (acc, x) -> acc + x) // 101010

// //
// // Seq.count
// //
// print [].count(fn (x) -> x) // 0
// print [1,2,3].count(fn (x) -> x) // 0 (bc fn does not return a boolean)
// print [1,2,3].count(fn (x) -> x > 1) // 2
// print [1,2,3].count(fn (x) -> x > 1 and x < 3) // 1

// //
// // Seq.concat
// //
// print [].concat([]) // []
// print [1,2,3].concat([]) // [1, 2, 3]
// print [].concat([1,2,3]) // [1, 2, 3]
// print [1,2,3].concat([4,5,6]) // [1, 2, 3, 4, 5, 6]

// print "--------------------------------------------------------------------------------"
// print "Modules"
// print "--------------------------------------------------------------------------------"

// // import std // Looks for "{cwd}/std.sl"
// // import std from "/modules/std"  // Relative path. Looks for "{cwd}/modules/std.sl"
// // import std from "C:/Projects/slang/modules/std.sl" // Absolute path
// // import std from "modules/std"

// // Bound native functions
// print "Bound native function test: "
// let sample = "Hello"
// let bound_native = sample.to_str
// print typeof(bound_native)
// print "'" + bound_native() + "' should be the same as '" + (sample.to_str)() +  "', and the same as '" + sample

// print "--------------------------------------------------------------------------------"
// print "Overriding stuff"
// print "--------------------------------------------------------------------------------"

// cls ListWithoutStrOverride {
//   ctor {
//     this.list = [1,2,[true, nil, "lol"]]
//   }
// }

// cls List {
//   ctor {
//     this.list = [1,2,[true, nil, "lol"]]
//   }

//   fn to_str {
//     ret "List with " + this.list.len + " elements: " + this.list
//   }
// }

// print ListWithoutStrOverride()
// print List()

// let l = List()
// log("Hello", [1,2,3], clock(), typeof(nil), l.list.len, typeof(l.list))

// print "--------------------------------------------------------------------------------"
// print "Builtin stuff"
// print "--------------------------------------------------------------------------------"

// import std from "/modules/std"
// cls Sample { ctor { this.value = 10 } }
// fn sample_fn { ret 10 }

// print Sample.__name
// print Sample.ctor

// let values = [0, nil, true, [], fn->1, clock, Sample]
// for let i = 0 ; i < values.len; i++ ; {
//   log("---")
//   log("typeof:   ", typeof(values[i]))
//   log("to_str:    ", values[i])
//   log("hash:      ", values[i].hash())
// }

// print cwd()
// print std.__file_path

// print "--------------------------------------------------------------------------------"
// print "Objs"
// print "--------------------------------------------------------------------------------"

// let f = fn -> 10
// let s = [1,2,3]

// let a = {
//     1: 2, 
//     true: false,
//     nil: nil, 
//     "hello": "world",
//     fn -> 1: fn -> 2, // Unreachable, because we have no reference to this function
//     f: fn -> 2,       // Reachable
//     [1,2,3]: [4,5,6], // Unreachable, because we have no reference to this array
//     s: [4,5,6]        // Reachable
// }

// print a[fn -> 1] // Should print: nil
// print a[[1,2,3]] // Should print: nil
// print a[f] // Should print: <Fn __anon>
// print a[f]() // Should print: 2
// print a[s] // Should print: [4, 5, 6]
// print a["hello"] // Should print: "world"
// print "-------"
// print a.keys()
// print a.values()
// print a.entries()
// print "-------"

// // Order is not guaranteed, since the underlying data structure is a hash table
// print a // Should print: {[1, 2, 3]: [4, 5, 6], [1, 2, 3]: [4, 5, 6], <Fn __anon>: <Fn __anon>, <Fn __anon>: <Fn __anon>, nil: nil, true: false, 1: 2, hello: world}

// print "1234\n".split()
// print "1234\n4321".split("4")
// print "123".split("")

// print "--------------------------------------------------------------------------------"
// print "File, Gc & Perf modules"
// print "--------------------------------------------------------------------------------"

// import File
// import Perf
// import Gc

// // Reading a file & measuring time
// let start = Perf.now()
// let lines = File.read(cwd() + "sample.sl").split("\n")
// let code = lines.filter(fn (line) -> line.len > 2 and line[0] != "/" and line[1] != "/")
// let time = Perf.since(start) * 1000

// log(time.to_str() + "ms")
// code.each(fn (line,i) -> log(i, "| ", line))

// print ""

// // Writing to a file, checking if it exists
// let out_file = File.join_path(cwd(), "sample.result.txt")
// print File.exists(out_file)
// print File.write(out_file, time.to_str() + "s\n")

// // Getting gc stats and forcing a collection cycle
// let prev = Gc.stats()
// start = Perf.now()
// let freed = Gc.collect()
// time = Perf.since(start) * 1000
// let { bytes_allocated } = Gc.stats()

// print "Freed:     " + freed + " bytes"
// print "GC time:   " + time + "ms"
// print "Allocated: " + bytes_allocated + " bytes"
// print "Prev:      " + prev.bytes_allocated + " bytes"

// print "--------------------------------------------------------------------------------"
// print "Try/Catch, Try/Else and Throw"
// print "--------------------------------------------------------------------------------"

// // Basic try/catch statements
// try {
//   print "Level one" 
//   try {
//     print "Level two" 
//     throw "Thrown exception"
//   }
//   catch {
//     print "Caught in level two"
//     throw error // Context variable 'error' is available in catch blocks
//   }
// } 
// catch {
//   print("Caught in level one!")
// }

// // Statements are allowed, blocks aren't necessary
// try Derived().foo()
// catch print error

// // Try/Else is an expression and returns a value
// let a = try 1 + true else "nope"
// print a

// // No else clause will default to nil in the error case
// let b = try 1 + true
// print b 

// // Using the context variable 'error' is allowed
// let c = try 1 + true else error
// print c 
// print "still running" 

// // Nested try/else
// let x = try try ASLDJKASLDJK else error else error
// print x // Inner error
// print "still running..."

// print "--------------------------------------------------------------------------------"
// print "Is operator"
// print "--------------------------------------------------------------------------------"

// print nil is Nil
// print true is Bool
// print 1 is Num
// print "Hello" is Str
// print [1,2,3] is Seq
// print {1:2, 3:4} is Obj
// print (fn -> 1) is Obj
// print (fn -> 1)() is Num
// print 1 == 2 is Bool
// print 1 < 2 is Bool

// // Only Classes are accepted as the rval of 'is' operator.
// print try (0 is 1) else error // Type must be a class. Was Num.
// print 0 is typeof(1)         // true

// print "--------------------------------------------------------------------------------"
// print "Fn.bind"
// print "--------------------------------------------------------------------------------"
// cls Lol {
//   ctor { this.y = 1 }
//   fn x -> this.y
//   static fn y -> 2
// }

// let new = {}
// let construct = Lol.ctor.bind(new)
// let get_x = Lol.x.bind(new)

// print construct()
// print get_x()

// import { Assert, Is, Does } from "/modules/test"

// Assert.that(true, Is.true_())
// Assert.that(false, Is.false_())
// Assert.that(nil, Is.nil_())
// Assert.that(1, Is.equal_to(1))
// Assert.that(1, Is.not_equal_to(2))

// Assert.that(fn {throw "Error"}, Does.throw_("Error"))
// Assert.that(fn {}, Does.not_throw())
// Assert.that(fn {throw "Error"}, Does.not_throw())

// // cls A {
// //   ctor {
// //     this.a = 1
// //     this.b = 2
// //   }
// // }

// // print A().entries() // [expect] [[a, 1], [b, 2]]
// // print A()["a"]      // [expect] 1
// // print A()["b"]      // [expect] 2  b

// import { Set } from "/modules/std"

// let set = Set()
// set.add(1)
// set.add(2)

// let other = Set()
// other.add(2)
// other.add(3)

// let union = set.union(other)
// print union

// // Test intersection
// let intersection = set.intersection(other)
// print intersection

// // Test difference
// let difference = set.difference(other)
// print difference

// // Test symmetric_difference
// let symmetric_difference = set.symmetric_difference(other)
// print symmetric_difference


cls Iterator {
  ctor (listlike) {
    if typeof(listlike) in [Seq, Tuple] { 
      this.ref = listlike
      this.index = 0
    } else if listlike is Obj {
      this.ref = listlike.entries()
      this.index = 0
    } else {
      throw "Expected a Seq, Tuple or Obj but got " + typeof(listlike).to_str() + " instead."
    }
  }

  fn next {
    if this.index >= this.ref.len ret this // Return self to indicate that the iterator is exhausted
    let res = this.ref[this.index]
    this.index = this.index + 1
    ret res
  }
}

let seq_iter = Iterator([1,2,3])

print seq_iter.next() // [expect] 1
print seq_iter.next() // [expect] 2
print seq_iter.next() // [expect] 3
print seq_iter.next() // [expect] <Instance of Iterator>

const obj = {"a": 1, "b": 2, "c": 3}
const obj_iter = Iterator(obj)

print obj_iter.next() // [expect] [a, 1]
print obj_iter.next() // [expect] [b, 2]
print obj_iter.next() // [expect] [c, 3]
print obj_iter.next() // [expect] <Instance of Iterator>

const tuple = (1, 2, 3)
const tuple_iter = Iterator(tuple)

print tuple_iter.next() // [expect] 1
print tuple_iter.next() // [expect] 2
print tuple_iter.next() // [expect] 3
print tuple_iter.next() // [expect] <Instance of Iterator>

// Passing a non-iterable value
print try Iterator(1) else error // [expect] Expected a Seq, Tuple or Obj but got Int instead.