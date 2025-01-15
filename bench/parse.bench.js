import fs from "fs";
import path from "path";
import url from "url";

const __dir = path.dirname(url.fileURLToPath(import.meta.url));

const digits = "0123456789";
const prg = fs.readFileSync(path.join(__dir, "parse.bench.input"), "utf8");

function run(do_enable) {
  let i = 0;
  let res = 0;
  let enable = true;

  function is_digit() {
    return digits.includes(prg[i]);
  }

  function check(str) {
    return prg.slice(i, i + str.length) === str;
  }

  function match(str) {
    if (!check(str)) return false;
    i += str.length;
    return true;
  }

  function num() {
    let n = "";
    while (is_digit()) {
      n += prg[i];
      i++;
    }
    return parseInt(n);
  }

  while (i < prg.length) {
    if (do_enable && match("don't()")) enable = false;
    if (do_enable && match("do()")) enable = true;

    if (match("mul(")) {
      const a = num();
      if (!match(",")) continue;
      const b = num();
      if (!check(")")) continue;
      if (enable) res += a * b;
    }
    i++;
  }
  return res;
}

const start = process.hrtime.bigint();
console.log(run(false)); // 168539636
console.log(run(true)); // 97529391
console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
