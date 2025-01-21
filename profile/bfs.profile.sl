import File
import Perf

const map = File
  .read(cwd() + "/bfs.profile.input")
  .split("\r\n")

const start = Perf.now()

const ROWS = map.len
const COLS = map[0].len
const DIRS = [ (-1, 0), (0,1), (1,0), (0,-1) ]

const prices1 = []
const prices2 = []

const SEEN = {}
for let y = 0; y < ROWS; ++y; {
  for let x = 0; x < COLS; ++x; {
    if (y,x) in SEEN skip

    const type = map[y][x]
    let field = {}
    let fences = {} // map of all directions to the set of field-positions that have a fence in that direction
    let perimeter = 0

    DIRS.each(fn(dir) -> fences[dir] = {})

    // field-floodfill
    const Q = [(y,x)]
    while Q.len > 0 {
      const pos = Q.yank(0)
      const (y,x) = pos

      if (map[y][x] != type) or (pos in field) skip
      field[pos] = true
      SEEN[pos] = true

      DIRS.each(fn(dir) {
        const (dy,dx) = dir
        const ny = y + dy
        const nx = x + dx
        if (ny>=0 and ny<ROWS) and (nx>=0 and nx<COLS) and map[ny][nx]==type {
          Q.push((ny,nx)) // in map & same crop-type
        } else {
          ++perimeter
          fences[dir][pos] = true 
        }
      })
    }

    // count sides
    let sides = 0
    fences.entries().each(fn(entry) {
      const fields = entry[1]
      const SEEN2 = {}
      fields.keys().each(fn(field) {
        if !(field in SEEN2) {
          ++sides

          const Q = [field]
          while Q.len > 0 {
            const pos = Q.yank(0)
            
            if pos in SEEN2 skip
            SEEN2[pos] = true

            DIRS.each(fn(dir) {
              const npos = (pos[0]+dir[0], pos[1]+dir[1])
              if (npos in fields) and !(npos in SEEN2) {
                Q.push(npos)
              }
            })
          }
        }
      })
    })

    prices1.push(perimeter * field.len)
    prices2.push(sides * field.len)
  }
}

print prices1.sum() // 1533644
print prices2.sum() // 936718
print "elapsed: " + Perf.since(start) + "s"
