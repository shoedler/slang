// fn x {
//   ret 1 + "2"
// }
// print x()                                   // '1 + "2"' is offending


// let a = [1,2,3][4][0].map(fn { ret x + 1 }) // '[0]' is offending


// let a = [1,2,3][4].map(fn { ret x + 1 })    // '.map(...)' is offending


// let a = [1,2,3]
// nil.push(10)                                // '.push(...)' is offending


// import Perf
// fn fib(n) {
//     if n <= 1 ret n
//     ret fib(n - 1) + fib(n - 2)
// }
// let start = Perf.now()
// for let i = 0; i < 1; i++; {
//     print fib(30)
// }
// print "elapsed: " + 1 + (Perf.now() - start).to_str() + "s"  // ' + 1' is offending


// fn foo(a) {
//     print a
// }
// foo()                                           // '()' is offending


// fn stack_ovfl() {
//     stack_ovfl()
// }
// stack_ovfl()                                    // '()' is offending


// cls X {
// }
// X().foo()                                       // 'foo()' is offending


// import Gaga                                     // 'import Gaga' is offending


// Obj.__name = "lol"                              // '__name = "lol"' is offending


// [1][1.0] = 0                                    // '[1.0] = 0' is offending


// let x = 2
// let y = 3
// print x + y + z + 1                             // 'z' is offending


// cls A { }
// cls B : A {
//     ctor { base.foo }
// }
// print B()                                        // 'foo' is offending


// print -"1000"                                    // '-"1000"' is offending

// let B = nil
// cls A : B { }                                    // 'B' is offending


// 2 is 1                                           // '1' is offending


// 2 in 1                                           // '1' is offending


// nil[1..2]                                        // '[1..2]' is offending

