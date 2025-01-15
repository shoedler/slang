// Note: I needed to raise the max stack frames from 64 to 128 when I originally solved this. 
//       This change didn't get committed bc it might have space complexity implications that I didn't investigate.

// [skip] Stack overflow, 64 is not enough & I don't want to raise the limit. Yet.

import File
import Gc

Gc.stress(false) // ⚠️ Disable stress mode which is enabled by default for testing - otherwise this will take almost forever to run

const source = File
  .read(cwd() + "/fuzzy-aoc-2024-day-11.txt")
  .split(" ")
  .map(Int)

const DP = {}
fn blink(stone, blinks) {
  let ans = 0
  if (stone, blinks) in DP ret DP[(stone, blinks)]

  if blinks == 0 ans = 1
  else if stone == 0 ans = blink(1, blinks-1)
  else if stone.to_str().len %2 == 0 {
    const str = stone.to_str()
    const first = Int(str[0..Int(str.len/2)])
    const second = Int(str[Int(str.len/2)..str.len])
    ans = blink(first, blinks-1) + blink(second, blinks-1)
  }
  else ans = blink(stone * 2024, blinks-1)

  DP[(stone, blinks)] = ans
  ret ans
}

log("Part 1:", source.map(fn(stone) -> blink(stone, 25)).sum()) // [expect] Part 1: 193899
log("Part 2:", source.map(fn(stone) -> blink(stone, 75)).sum()) // [expect] Part 2: 229682160383225

