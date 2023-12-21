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
// for let i = 0 ; i < 5 ; i = i + 1 {
//   print i
// }

// let b = false
// for b ; !b { // Incrementing is optional
//   print b
//   b = true
// }

// let c = 0
// while b {
//   c = c + 1
//   b = c < 10
//   print c
// }

// for let i = 0 ; i < 100 ; i = i + 1 {
//   let outer = fn -> {
//     let x = "outside"
//     let inner = fn -> {
//       print x
//     }
//     ret inner;
//   }

//   for let y = 0; y < 100; y = y + 1 {
//     let x = fn -> {
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
// let a = fn -> print "a" // Anonymous Fn
// let a_args = fn x,y,z -> print x + y + z // Anonymous Fn
// fn b -> print "b" // Named Fn
// fn b_args x,y,z -> print x + y + z // Named Fn

// print a
// print a_args
// print b
// print b_args

// // -----------------------------------------
// // Functions can be recursive
// // -----------------------------------------
// let fib = fn n ->
//   if n <= 1
//     ret n;
//   else
//     ret (fib(n-1) + fib(n-2));

// // With native functions. Here, timed with clock()
// let start = clock()
// print "Let's calculate fib(35)!"
// let result = fib(35)
// print clock() - start

// print result

// +----------------------------+------------------------------------------+--------+---------+-----------+
// | CHANGE                     | COMMIT HASH                              | FIB #  | t DEBUG | t RELEASE |
// +----------------------------+------------------------------------------+--------+---------+-----------+
// | After adding classes       | 03374177fea58b99ab4a1c47014aca8855a50485 | 35     | 6.732s  | 0.837s    |
// +----------------------------+------------------------------------------+--------+---------+-----------+
// | After book + optimizations | 6e4ac55250f254043d6049bfaef5476ca71abea0 | 35     | 6.589s  | 0.819s    |
// +----------------------------+------------------------------------------+--------+---------+-----------+

// // -----------------------------------------
// // Functions have / are Closures
// // -----------------------------------------
// fn outer -> {
//   let x = "outside"
//   let inner = fn -> {
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
//   fn topping = first, second -> {
//     print "scone with " + first + " and " + second
//     ret 1;
//   }
// }

// let scone = Scone()
// let res = scone.topping("berries", "cream")
// print res
// print scone.topping

// cls Nested {
//   fn method -> {
//     let function = fn -> {
//       print this
//     }

//     function()
//   }
// }

// Nested().method() // Prints "Nested Instance"

// cls CoffeeMaker {
//   ctor = coffee -> {
//     this.coffee = coffee
//   }

//   fn brew -> {
//     print "Enjoy your cup of " + this.coffee

//     // No reusing the grounds!
//     this.coffee = nil
//   }
// }

// let maker = CoffeeMaker("coffee and chicory")
// maker.brew() 
// // maker.brew() // Error

// // -----------------------------------------
// // Base Classes
// // -----------------------------------------
// cls A {
//   fn method -> {
//     print "A"
//   }
// }

// while false {
//   let a = nil
// }

// cls B : A {
//   fn method -> {
//     let closure = base.method
//     closure() // Prints "A"
//   }
// }

// let b = B()
// b.method()

// -----------------------------------------
// Benchmarks
// -----------------------------------------
cls Zoo {
  ctor {
    this.aardvark = 1
    this.baboon   = 1
    this.cat      = 1
    this.donkey   = 1
    this.elephant = 1
    this.fox      = 1
  }

  fn ant    -> this.aardvark
  fn banana -> this.baboon
  fn tuna   -> this.cat
  fn hay    -> this.donkey
  fn grass  -> this.elephant
  fn mouse  -> this.fox
}

let sum = 0
let duration = 1
let batches = 0

let start = clock()
let i = 0

while clock() < start + duration {
  while i < 10000 {
    let zoo = Zoo()
    sum = sum + zoo.ant()
            + zoo.banana()
            + zoo.tuna()
            + zoo.hay()
            + zoo.grass()
            + zoo.mouse()
    i = i + 1
  }
  batches = batches + 1
  i = 0
}

print "Sum: " 
print sum
print "Batches of 10000: "
print batches
print "In"
print duration
print "seconds"

fn func(x) -> x

let anon = fn -> 123
let anon2 = fn(x) {
  print x
}

cls A {
  ctor {
    this.a = 1
  }

  fn method(y) {
    print this.a
    print y
  }
}

let a = A()
a.method(78)

print func(90)
print func

print anon()
print anon

print anon2(90)
print anon2