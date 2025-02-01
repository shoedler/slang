import Perf

fn fib(n) {
    if n <= 1 ret n
    ret fib(n - 1) + fib(n - 2)
}

let start = Perf.now()
for let i = 0; i < 5; ++i; {
    print fib(30)
}

print "elapsed: " + (Perf.now() - start) + "s"