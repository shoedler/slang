// print "--------------------------------------------------------------------------------"
// print "Conditionals"
// print "--------------------------------------------------------------------------------"
// let a = 123
// if a a = 321
// print a // 321

// let b = "Hi"
// if !b b = "?" else b = b + " Wrld"
// print b // "Hi Wrld"

// if a and b print "a and b is Truthy!"
// if a or b print "a or b is Truthy!"

// print "--------------------------------------------------------------------------------"
// print "Loops"
// print "--------------------------------------------------------------------------------"
// for let i = 0 ; i < 5 ; i = i + 1 ; {
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

// for let i = 0 ; i < 100 ; i = i + 1 ; {
//   let outer = fn {
//     let x = "outside"
//     let inner = fn {
//       print x
//     }
//     ret inner;
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
// let fib = fn (n) -> n <= 1 and n or fib(n-1) + fib(n-2)

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
//     ret 1;
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
// print "Base Classes"
// print "--------------------------------------------------------------------------------"
// cls A {
//   fn method {
//     print "A"
//   }
// }

// while false {
//   let a = nil
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

// // print sys.clock()
// print "Hello"
// print 123.type
// print "Hello".len
// // print "Hello".get(0)

// fn is_num(x) {
//   ret type_of(x) == Num;
// }

// cls Validators { 
//   ctor { this.num = 3 }
//   fn is_num_3(x) { ret x == this.num; }
// }

// let validators = Validators()

// print [1,3,4].has(2)
// print [1,3,4].has(3)
// print [1,3,4].has(is_num)
// print [1,3,4].has(validators.is_num_3)
// validators.num = 2
// print [1,3,4].has(validators.is_num_3)

// print [].has.__doc

print [].first() // nil
print [1,2,3].first() // 1
print [1,2,3].first(fn (x) -> x/2 == 1) // 2

print [].last() // nil
print [1,2,3].last() // 3
print [1,2,3].last(fn (x) -> type_of(x) == Num) // 3

// print "--------------------------------------------------------------------------------"
// print "Modules"
// print "--------------------------------------------------------------------------------"

// // import std // Looks for "cwd/std.sl"
// import std from "/modules/std" 
// // import std from "C:/Projects/slang/modules/std.sl"
// // import std from "modules/std"

// let Range = std.Range
// let Monad = std.Monad

// // let rng = Range.__ctor(0, 10) // Does not work, because __ctor is not bound to Range if you call it like this
// let rng = Range(0, 5)

// let iter = rng.__iter() // Create a new iterator
// let res
// while res = iter() {
//   print res
// }

// let mnd = Monad(10)
// print mnd
//   .bind(fn(x) -> x + 1)
//   .bind(fn(x) -> x * 2)
//   .bind(fn(x) -> x - 1)
//   .value

// // Bound native functions
// print "Bound native function test: "
// let sample = "Hello"
// let bound_native = sample.to_str
// print type_name(bound_native)
// print "'" + bound_native() + "' should be the same as '" + (sample.to_str)() +  "', and the same as '" + sample.to_str()

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
//     ret "List with " + this.list.len().to_str() + " elements: " + this.list.to_str();
//   }
// }

// print ListWithoutStrOverride().to_str()
// print List().to_str()

// let l = List()
// log("Hello", [1,2,3], clock(), type_name(nil), l.list.len(), type_name(l.list))

// print "--------------------------------------------------------------------------------"
// print "Builtin stuff"
// print "--------------------------------------------------------------------------------"

// import std from "/modules/std"
// cls Sample { ctor { this.value = 10 } }
// fn sample_fn { ret 10; }

// print Sample.__name
// print Sample.__ctor

// let values = [0, nil, true, [], fn->1, clock, Sample]
// for let i = 0 ; i < values.len(); i = i + 1 ; {
//   log("---")
//   log("type_name: ", type_name(values[i]))
//   log("type_of:   ", type_of(values[i]))
//   log("to_str:    ", values[i].to_str())
//   log("hash:      ", values[i].hash())
// }

// print cwd()
// print std.__file_path

// print Nil.__ctor.__doc
// print Bool.__ctor.__doc
// print Num.__ctor.__doc
// print Str.__ctor.__doc
// print Seq.__ctor.__doc
// print Map.__ctor.__doc

// // These have no docstrings:
// print "".to_str.__doc
// print "".len.__doc
// print (fn->1).__doc
// print sample_fn.__doc
// print [1,23].__doc
// print Sample.__doc
// print Sample().__doc

// print clock.__doc
// print type_name.__doc
// print type_of.__doc
// print cwd.__doc
// print log.__doc

// print "--------------------------------------------------------------------------------"
// print "Maps"
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