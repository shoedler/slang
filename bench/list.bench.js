const start = process.hrtime();
const list = [];
for (let i = 0; i < 1000000; i++) {
  list.push(i);
}

let sum = 0;
for (let i = 0; i < list.length; i++) {
  sum += list[i];
}
console.log(sum);

console.log(`elapsed: ${(process.hrtime(start)[1] / 1e9).toFixed(5)}ms`);
