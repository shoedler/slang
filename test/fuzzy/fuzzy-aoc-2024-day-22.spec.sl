import File
import Math
import Gc

Gc.stress(false) // ⚠️ Disable stress mode which is enabled by default for testing - otherwise this will take almost forever to run 

const secret_nums = File
  .read(cwd() + "/fuzzy-aoc-2024-day-22.txt")
  .split(File.newl)
  .map(fn(line) -> Int(line))

fn prune(x) -> x%16777216
fn mix(x,y) -> Math.xor(x,y)

fn secrets(x) -> [x].concat(Seq(2000).map(fn(i) {
  x = prune(mix(x, x * 64))
  x = prune(mix(x, Math.floor(x / 32)))
  x = prune(mix(x, x * 2048))
  ret x
}))

fn changes(p) -> p[1..].map(fn(x, i) -> x - p[i])

fn seqs(prices, changes) {
  const SEEN = {}
  for let i=0; i<changes.len-3; i++; {
    const seq = changes[i..i+4].join(",") // Found a bug when hashing tuples with negative values, so I'm using a string here
    if seq in SEEN skip
    SEEN[seq] = prices[i+4]
  }
  ret SEEN.entries()
}

let last_secrets = 0
const SCORE = {}

secret_nums.each(fn(num) {
  const all = secrets(num)
  last_secrets += all[-1]
  const prices = all.map(fn(x) -> x%10) // 🍌
  seqs(prices, changes(prices)).each(fn(sp) {
    const [seq, price] = sp
    if seq in SCORE SCORE[seq] += price
    else SCORE[seq] = price
  })
})

log("Part 1", last_secrets) // [expect] Part 1 37990510
log("Part 2", SCORE.values().sort()[-1]) // [expect] Part 2 23

// (⚠️ Runs on example input to make it faster)
