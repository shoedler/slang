// [skip] Too slow

import File

const eqs = File
  .read(cwd() + "/fuzzy-aoc-2024-day-07.txt")
  .split("\r\n")
  .map(fn(l) {
    const [res, rest] = l.split(":")
    const nums = rest
      .trim()
      .split(" ")
      .map(Int)
      
    ret [Int(res), nums]
  })

const ops = [
  fn(a, b) -> a+b,
  fn(a, b) -> a*b,
]

fn combinations(nums) {
  const rs = []

  fn calc(pos, val, rest) -> pos < nums.len-1 ?
    ops.each(fn(op) -> calc(pos+1, op(val, nums[pos+1]), rest)) :
    rs.push(val)

  calc(0, nums[0], nums[1..])
  ret rs
}

fn solve -> eqs
  .map(fn(eq) -> (eq[0] in combinations(eq[1])) ? eq[0] : 0)
  .sum()

log("Part 1:", solve()) // [expect] Part 1: 3749
ops.push(fn(a, b) -> Int(a.to_str()+b))
log("Part 2:", solve()) // [expect] Part 2: 11387

// (⚠️ Runs on example input to make it faster)