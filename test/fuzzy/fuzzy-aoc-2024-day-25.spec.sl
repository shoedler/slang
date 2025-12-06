import File
import Math
import Gc

Gc.stress(false) // ⚠️ Disable stress mode which is enabled by default for testing - otherwise this will take almost forever to run 

const [LOCKS, KEYS] = File
  .read(cwd() + "/fuzzy-aoc-2024-day-25.txt")
  .split(File.newl + File.newl)
  .fold([[],[]], fn(acc, lock_or_key) {
    const flat = lock_or_key
      .split(File.newl)
      .fold([], fn(flat, line) {
        line.ascii().each(fn(code) -> flat.push(code))
        ret flat
      })

    if lock_or_key[0] == "#" acc[0].push(flat)
    else acc[1].push(flat)
    ret acc
  })

const VOID = ".".ascii()[0]
const PIN = "#".ascii()[0]

fn void_on_void(a,b) -> a-b == 0 and a == VOID
fn pin_on_void(a,b) -> Math.abs(a-b) == VOID-PIN
fn match(a,b) -> void_on_void(a,b) or pin_on_void(a,b)

log("Part 1", LOCKS.fold(0, fn(res, lock) -> res + KEYS.count(fn(key) -> lock.every(fn(code,i) -> match(code, key[i]))))) // [expect] Part 1 2933
