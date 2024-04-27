const start = process.hrtime.bigint();

for (let i = 0; i < 10000; i++) {
  let a = (1000).toString();
  let b = (1.1245).toString();
  let c = "hello".toString();
  let d = true.toString();
  let e = String(null);
  let f = [1, 2, 3].toString();
  let g = { a: 1, b: 2 }.toString();
  const h = a + b + c + d + e + f + g;
}

console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
