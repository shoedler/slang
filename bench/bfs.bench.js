import fs from "fs";
import path from "path";
import url from "url";

const __dir = path.dirname(url.fileURLToPath(import.meta.url));

const mapData = fs
  .readFileSync(path.join(__dir, "bfs.bench.input"), "utf8")
  .split("\n")
  .map((line) => line.trim().split(""));

const start = process.hrtime.bigint();

const ROWS = mapData.length;
const COLS = mapData[0].length;
const DIRS = [
  [-1, 0],
  [0, 1],
  [1, 0],
  [0, -1],
];

const prices1 = [];
const prices2 = [];

const SEEN = new Set();
for (let y = 0; y < ROWS; y++) {
  for (let x = 0; x < COLS; x++) {
    const pos = `${y},${x}`;
    if (SEEN.has(pos)) {
      continue;
    }

    const cropType = mapData[y][x];
    const field = new Set();
    const fences = {}; // Map direction -> set of positions
    let perimeter = 0;

    // Initialize fence sets for each direction
    for (const d of DIRS) {
      fences[d] = new Set();
    }

    // field-floodfill
    const Q = [`${y},${x}`];
    while (Q.length > 0) {
      const curPos = Q.shift();
      const [cy, cx] = curPos.split(",").map(Number);

      if (mapData[cy][cx] !== cropType || field.has(curPos)) {
        continue;
      }

      field.add(curPos);
      SEEN.add(curPos);

      for (let dirIdx = 0; dirIdx < DIRS.length; dirIdx++) {
        const [dy, dx] = DIRS[dirIdx];
        const ny = cy + dy;
        const nx = cx + dx;
        if (
          ny >= 0 &&
          ny < ROWS &&
          nx >= 0 &&
          nx < COLS &&
          mapData[ny][nx] === cropType
        ) {
          Q.push(`${ny},${nx}`);
        } else {
          perimeter++;
          fences[DIRS[dirIdx]].add(curPos); // Store with specific direction
        }
      }
    }

    // count sides
    let sides = 0;
    for (const dirKey in fences) {
      const fields = fences[dirKey];
      const SEEN2 = new Set();

      for (const fieldPos of fields) {
        if (!SEEN2.has(fieldPos)) {
          sides++;

          const Q = [fieldPos];
          while (Q.length > 0) {
            const pos = Q.shift();

            if (SEEN2.has(pos)) {
              continue;
            }
            SEEN2.add(pos);

            // Check neighbors in all directions
            for (const checkDir of DIRS) {
              const [py, px] = pos.split(",").map(Number);
              const npos = `${py + checkDir[0]},${px + checkDir[1]}`;
              if (fields.has(npos) && !SEEN2.has(npos)) {
                Q.push(npos);
              }
            }
          }
        }
      }
    }

    prices1.push(perimeter * field.size);
    prices2.push(sides * field.size);
  }
}

console.log(prices1.reduce((a, b) => a + b, 0));
console.log(prices2.reduce((a, b) => a + b, 0));
console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
