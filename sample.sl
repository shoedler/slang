// // -----------------------------------------
// // Conditionals
// // -----------------------------------------
// let a = 123
// if a a = 321
// print a // 321

// let b = "Hi"
// if !b b = "?" else b = b + " Wrld"
// print b // "Hi Wrld"

// if a and b print "a and b is Truthy!"
// if a or b print "a or b is Truthy!"

// // -----------------------------------------
// // Loops
// // -----------------------------------------
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

// -----------------------------------------
// Functions
// -----------------------------------------
// let a = fn -> "a" // Anonymous Fn
// let a_args = fn(x,y,z) -> x + y + z // Anonymous Fn
// fn b -> "b" // Named Fn
// fn b_args(x,y,z) -> x + y + z // Named Fn

// print a
// print a_args
// print b
// print b_args

// // -----------------------------------------
// // Functions can be recursive
// // -----------------------------------------
// let fib = fn (n) -> n <= 1 and n or fib(n-1) + fib(n-2)

// // With native functions. Here, timed with clock()
// let start = clock()
// print "Let's calculate fib(35)!"
// let result = fib(35)
// print clock() - start

// print result

// // -----------------------------------------
// // Functions have / are Closures
// // -----------------------------------------
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

// // -----------------------------------------
// // Classes 
// // -----------------------------------------
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

// // -----------------------------------------
// // Base Classes
// // -----------------------------------------
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

// -----------------------------------------
// Sequences
// -----------------------------------------
let x = [1,2,3]
print x
print x[0]

x[0] = 4
print x[0]
x[1] = ["Foo", "Bar"]
print x

cls Seq {
  ctor {
    this.list = [1,2,3]
  }

  fn printList {
    print this.list
  }
}

fn make_list -> [1,2,3]
let s = Seq()
s.printList()
s.list[1] = -2
s.printList()

print make_list()[2]
