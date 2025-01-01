import Math
import File
import Gc

Gc.stress(false) 

const grid = File
  .read(cwd() + "/fuzzy-aoc-2024-day-08.txt")
  .split("\r\n")
  .map(fn(row) -> row.split(""))

const antennas = {}
grid.each(fn(row, y) -> 
  row.each(fn(freq, x) {
    if freq == "." ret

    if antennas[freq] 
      antennas[freq].push((y,x))
    else
      antennas[freq] = [(y,x)]
  })
)

const ROWS = grid.len
const COLS = grid[0].len

fn inbound(y, x) -> y >= 0 and y < ROWS and x >= 0 and x < COLS

// Model 1
const antinodes_1 = {}
fn antinode_1(a, b) {
  const delta_y = b[0]-a[0]
  const delta_x = b[1]-a[1]

  const cy = a[0] - delta_y
  const cx = a[1] - delta_x

  const dy = b[0] + delta_y
  const dx = b[1] + delta_x

  if inbound(cy, cx) antinodes_1[(cy, cx)] = true
  if inbound(dy, dx) antinodes_1[(dy, dx)] = true
}

// Model 2
const antinodes_2 = {}
fn antinode_2(a, b) {
  const delta_y = b[0]-a[0]
  const delta_x = b[1]-a[1]

  let y = a[0]
  let x = a[1]
  while inbound(y, x) {
    antinodes_2[(y, x)] = true
    y += delta_y
    x += delta_x
  }

  y = a[0]
  x = a[1]
  while inbound(y, x) {
    antinodes_2[(y, x)] = true
    y -= delta_y
    x -= delta_x
  }
}

antennas.values().each(fn(list) {
  for let i = 0; i < list.len-1; ++i; {
    for let j = i+1; j < list.len; ++j; {
      const a = list[i]
      const b = list[j]
      antinode_1(a, b)
      antinode_2(a, b)
    }
  }
})

log("Part 1", antinodes_1.len) // [expect] Part 1 305
log("Part 2", antinodes_2.len) // [expect] Part 2 1150
