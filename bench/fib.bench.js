function fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

const start = process.hrtime.bigint();
for (let i = 0; i < 5; i++) {
  console.log(fib(30));
}

console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
