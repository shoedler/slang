import File
import Gc

Gc.stress(false) // ⚠️ Disable stress mode which is enabled by default for testing - otherwise this will take almost forever to run

const [towels, designs] = File
  .read(cwd() + "/fuzzy-aoc-2024-day-19.txt")
  .split("\r\n\r\n")

const TOWELS = towels.split(",").map(fn(x) -> x.trim())
const DESIGNS = designs.split("\r\n").map(fn(x) -> x.trim())

const DP = {}
fn combos(design) {
  if design in DP ret DP[design]
  let ways = design.len == 0 ? 1 : 0

  for let i = 0; i < TOWELS.len; i++; {
    const towel = TOWELS[i]
    const start = design[..towel.len]
    if start == towel 
      ways += combos(design[towel.len..])
  }

  DP[design] = ways
  ret ways
}

const ways = DESIGNS.map(fn(d) -> combos(d))
log("Part 1", ways.sift(fn(c) -> c>0).len) // [expect] Part 1 315
log("Part 2", ways.sum()) // [expect] Part 2 625108891232249
