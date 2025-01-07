import { readFileSync } from "fs";

const input = readFileSync(process.cwd() + "/bfs-dp.bench.input", "utf8");
const grid = input.split("\r\n").map((l) => l.split(""));

const ROWS = grid.length;
const COLS = grid[0].length;

function findStart() {
  for (let i = 0; i < ROWS; i++) {
    const x = grid[i].indexOf("^");
    if (x !== -1) return [i, x];
  }
}

const start = findStart();
const DIRS = [
  [-1, 0],
  [0, 1],
  [1, 0],
  [0, -1],
];

const DP = new Map();
function trace(oy, ox) {
  const key = `${oy},${ox}`;
  if (DP.has(key)) return DP.get(key);

  const loop = new Map();
  const seen = new Map();

  let [y, x] = start;
  let d = 0;

  while (true) {
    const key = `${y},${x},${d}`;
    if (loop.has(key)) {
      DP.set(`${oy},${ox}`, true);
      return true;
    }

    loop.set(key, true);
    seen.set(`${y},${x}`, true);

    const [dy, dx] = DIRS[d];
    const ny = y + dy;
    const nx = x + dx;

    if (ny < 0 || ny >= ROWS || nx < 0 || nx >= COLS) {
      DP.set(`${oy},${ox}`, seen.size);
      return seen.size;
    }

    if (grid[ny][nx] === "#" || (oy === ny && ox === nx)) {
      d = (d + 1) % 4;
    } else {
      y = ny;
      x = nx;
    }
  }
}

const start_ = process.hrtime.bigint();
console.log(trace(null, null)); // 5239

let loops = 0;
for (let oy = 0; oy < ROWS; oy++) {
  for (let ox = 0; ox < COLS; ox++) {
    if (grid[oy][ox] === "#") continue;
    if (trace(oy, ox) === true) loops++;
  }
}

console.log(loops); // 1753
console.log(
  `elapsed: ${(
    Number(process.hrtime.bigint() - start_) / 1_000_000_000
  ).toFixed(5)}s`
);
