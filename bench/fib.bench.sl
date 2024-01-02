let fib = fn (n) -> n <= 1 and n or fib(n-1) + fib(n-2)

let start = clock()

// [LatencyBenchmark] Fib(35)
// [ExpectedValue] 9227465
print fib(35)          // [Value]
print clock() - start  // [DurationInSecs]
