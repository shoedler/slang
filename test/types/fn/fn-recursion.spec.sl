fn fib(n) {
  if (n < 2) ret n
  ret fib(n - 1) + fib(n - 2)
}

print fib(8) // [expect] 21
