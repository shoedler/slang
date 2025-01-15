import fs from "fs";
import path from "path";
import url from "url";

const __dir = path.dirname(url.fileURLToPath(import.meta.url));

// Read input file
const codes = fs
  .readFileSync(path.join(__dir, "dp.bench.input"), "utf8")
  .split("\n")
  .map((line) => line.trim());

const NUM_KEYS = ["789", "456", "123", " 0A"];

const DIR_KEYS = [" ^A", "<v>"];

const DIRS = { "^": [0, -1], ">": [1, 0], v: [0, 1], "<": [-1, 0] };

const start = process.hrtime.bigint();

function make_pad(pad) {
  // Map each key to its position on the keypad. E.g. "1" -> [0,2]
  const map = {};
  for (let y = 0; y < pad.length; y++) {
    for (let x = 0; x < pad[y].length; x++) {
      const key = pad[y][x];
      if (key !== " ") {
        map[key] = [x, y];
      }
    }
  }
  return map;
}

const num_keypad = make_pad(NUM_KEYS);
const dir_keypad = make_pad(DIR_KEYS);

function reps(str, n) {
  // Repeat a string n times
  let res = "";
  while (res.length < n) {
    res += str;
  }
  return res;
}

function unique_perms(str) {
  // Generate all unique permutations of a string
  if (str.length === 0) return [[]];
  if (str.length === 1) return [[str]];

  const perms = [];
  const chars = str.split("");

  function swap(i, j) {
    const tmp = chars[i];
    chars[i] = chars[j];
    chars[j] = tmp;
  }

  function permute(n) {
    if (n === 1) {
      perms.push([...chars]);
      return;
    }
    for (let i = 0; i < n; i++) {
      swap(i, n - 1);
      permute(n - 1);
      swap(i, n - 1);
    }
  }

  permute(chars.length);
  return perms;
}

const DP = {};
function calc_presses(seq, depth, dirkey, cur) {
  const DP_KEY = JSON.stringify([seq, depth, dirkey, cur]); // JS needs stringification for object keys
  if (DP_KEY in DP) return DP[DP_KEY];

  const keypad = dirkey ? dir_keypad : num_keypad;

  // Base cases
  if (seq.length === 0) return 0;
  if (cur === null) cur = keypad["A"];

  // Calc distance to the next key
  const [cx, cy] = cur;
  const [px, py] = keypad[seq[0]];
  const dx = px - cx;
  const dy = py - cy;

  // Generate the moves to get to the next key
  let moves = "";
  if (dx > 0 || dx < 0) moves += reps(dx > 0 ? ">" : "<", Math.abs(dx));
  if (dy > 0 || dy < 0) moves += reps(dy > 0 ? "v" : "^", Math.abs(dy));

  let min_len = moves.length + 1;
  if (depth > 0) {
    // Try all permutations of the moves to find the shortest path
    const valid_lens = [];
    unique_perms(moves).forEach((perm) => {
      let [cx, cy] = cur;
      let valid = true;

      for (const move of perm) {
        const [mdx, mdy] = DIRS[move];
        cx += mdx;
        cy += mdy;
        // Check if position is on keypad
        const on_keypad = Object.values(keypad).some(
          (pos) => pos[0] === cx && pos[1] === cy
        );
        if (!on_keypad) {
          valid = false;
          break;
        }
      }

      if (valid) {
        // Recurse on the direction keypad
        const path_len = calc_presses(
          perm.join("") + "A",
          depth - 1,
          true,
          null
        );
        if (path_len !== null) {
          valid_lens.push(path_len);
        }
      }
    });

    if (valid_lens.length > 0) {
      min_len = Math.min(...valid_lens);
    }
  }

  const res = min_len + calc_presses(seq.slice(1), depth, dirkey, [px, py]);
  DP[DP_KEY] = res;
  return res;
}

// Calculate results
const d2 = codes.reduce(
  (sum, code) =>
    sum + parseInt(code.slice(0, -1)) * calc_presses(code, 2, false, null),
  0
);

const d25 = codes.reduce(
  (sum, code) =>
    sum + parseInt(code.slice(0, -1)) * calc_presses(code, 25, false, null),
  0
);

console.log(d2); // 215374
console.log(d25); // 260586897262600
console.log(
  `elapsed: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(
    5
  )}s`
);
