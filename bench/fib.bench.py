import time

def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)


start = time.time()

# [LatencyBenchmark] Fib(35)
# [ExpectedValue] 9227465
print(fib(35))  # [Value]
print(time.time() - start)  # [DurationInSecs]
