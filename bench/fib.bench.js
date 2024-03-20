const fib = (n) => (n <= 1 ? n : fib(n - 1) + fib(n - 2));

const start = Date.now();

// [LatencyBenchmark] Fib(35)
// [ExpectedValue] 9227465
console.log(fib(35)); // [Value]
console.log(Date.now() - start); // [DurationInSecs]
