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

print "--------------------------------------------------------------------------------"
print "Modules"
print "--------------------------------------------------------------------------------"

import std
let Range = std.Range
let Monad = std.Monad

// let rng = Range.__ctor(0, 10) // Does not work, because __ctor is not bound to Range if you call it like this
let rng = Range(0, 5)

let iter = rng.__iter() // Create a new iterator
let res
while res = iter() {
  print res
}

let mnd = Monad(10)
print mnd
  .bind(fn(x) -> x + 1)
  .bind(fn(x) -> x * 2)
  .bind(fn(x) -> x - 1)
  .value

// Bound native functions
print "Bound native function test: "
let sample = "Hello"
let bound_native = sample.to_str
print type_name(bound_native)
print "'" + bound_native() + "' should be the same as '" + (sample.to_str)() +  "', and the same as '" + sample.to_str()

print "--------------------------------------------------------------------------------"
print "Overriding stuff"
print "--------------------------------------------------------------------------------"

cls ListWithoutStrOverride {
  ctor {
    this.list = [1,2,[true, nil, "lol"]]
  }
}

cls List {
  ctor {
    this.list = [1,2,[true, nil, "lol"]]
  }

  fn to_str {
    ret "List with " + this.list.len().to_str() + " elements: " + this.list.to_str();
  }
}

print ListWithoutStrOverride().to_str()
print List().to_str()

let l = List()
log("Hello", [1,2,3], clock(), type_name(nil), l.list.len(), type_name(l.list))

print "--------------------------------------------------------------------------------"
print "Builtin stuff"
print "--------------------------------------------------------------------------------"

print List.__name
print List.__ctor

print 312.to_str()
print 312.hash()
print type_name(321)
print type_of(312)
print "Hello".len()

let values = [0, nil, true, [], fn->1, clock, List]
for let i = 0 ; i < values.len(); i = i + 1 ; {
  log("---")
  log("type_name: ", type_name(values[i]))
  log("type_of:   ", type_of(values[i]))
  log("to_str:    ", values[i].to_str())
}

print cwd()
print std.__file_path
