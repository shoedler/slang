
import File
import Math

const grid = File
  .read(cwd() + "/fuzzy-aoc-2024-day-04.txt")
  .split("\r\n")

let p1 = 0
let p2 = 0

for let y =0; y < grid.len; y++; {
  for let x = 0; x < grid[y].len; x++; {
    // Horizontal (ltr & rtl)
    if x+3<grid[y].len and grid[y][x..x+4] in ["XMAS", "SAMX"] p1++
    // Vertical (ttb & btt)
    if y+3<grid.len and grid[y..y+4].map(fn (row) -> row[x]).join("") in ["XMAS", "SAMX"] p1++
    // Diagonal ttb (ltr & rtl)
    if y+3<grid.len and x+3<grid[y].len and grid[y..y+4].map(fn (row,i) -> row[x+i]).join("") in ["XMAS", "SAMX"] p1++
    // Diagonal btt (ltr & rtl)
    if y-3>=0 and x+3<grid[y].len and grid[y-3..y+1].flip().map(fn (row,i) -> row[x+i]).join("") in ["XMAS", "SAMX"] p1++

    // 0 . 4
    // . 1 .
    // 3 . 2
    if y+2<grid.len and x+2<grid[y].len {
      p2 += ["MASMS", "MASSM", "SAMMS", "SAMSM"].count(grid[y][x] + grid[y+1][x+1] + grid[y+2][x+2] + grid[y+2][x] + grid[y][x+2])
    }
  }
}

log("Part 1", p1) // [expect] Part 1 18
log("Part 2", p2) // [expect] Part 2 9

// (⚠️ Runs on example input to make it faster)
