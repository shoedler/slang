let fib = fn (n) -> n <= 1 and n or fib(n-1) + fib(n-2)

let start = clock()
print "Fib of 35:"
print fib(35)
print "Took (s):"
print clock() - start
