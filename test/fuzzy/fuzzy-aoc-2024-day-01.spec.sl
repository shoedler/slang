
import File
import Math

const nums = File
  .read(cwd() + "/fuzzy-aoc-2024-day-01.txt")
  .split("\r\n")
  .map(fn (line) -> line.split("   ").map(Int))

const L = nums.map(fn (p) -> p[0]).sort()
const R = nums.map(fn (p) -> p[1]).sort()

log("Part 1:", L.map(fn (l, i) -> Math.abs(l-R[i])).sum()) // [expect] Part 1: 1530215
log("Part 2:", L.map(fn (l) -> l * R.sift(fn (r) -> r == l).len).sum() ) // [expect] Part 2: 26800609