function fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

const start = process.hrtime();
for (let i = 0; i < 5; i++) {
  console.log(fib(30));
}

console.log(`elapsed: ${(process.hrtime(start)[1] / 1e9).toFixed(5)}ms`);
