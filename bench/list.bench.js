const start = process.hrtime.bigint();
const list = [];
for (let i = 0; i < 1000000; i++) {
  list.push(i);
}

let sum = 0;
for (let i = 0; i < list.length; i++) {
  sum += list[i];
}
console.log(sum);

console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
