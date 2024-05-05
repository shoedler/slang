let fib = fn (n) -> n <= 1 and n or fib(n-1) + fib(n-2)

let start = clock()
let result = fib(35)
let time = clock() - start

if result != 9227465 print "WRONG FIB(35) RESULT"
print time
