import File

const [grid_raw, moves_raw] = File
  .read(cwd() + "/sample.txt")
  .split("\r\n\r\n")

fn simulate(bigboxes) {
  const grid = []
  grid_raw
    .split("\r\n")
    .each(fn (line) {
      const row = []
      line.split("").each(fn(c) {
        if !bigboxes row.push(c)
        else {
          if c=="#" row.push("#","#")
          else if c=="O" row.push("[","]")
          else if c=="@" row.push("@",".")
          else row.push(c, ".")
        }
      })
      grid.push(row)
    })

  const moves = moves_raw
    .split("\r\n")
    .join("")

  const ROWS = grid.len
  const COLS = grid[0].len
  const DIRS = {"^":(-1, 0), ">":(0,1), "v":(1,0), "<":(0,-1)}

  // get start pos
  fn start_i(r) -> "@" in r
  fn not_nil(x) -> x != nil
  let robot = (
    grid.pos(start_i), 
    grid.map(fn (r) -> r.pos(start_i)).first(not_nil)
  )

  grid[robot[0]][robot[1]] = "."

  fn add(a,b) -> (a[0]+b[0], a[1]+b[1])
  fn at(pos) -> try grid[pos[0]][pos[1]] else nil
  fn set(pos, val) -> grid[pos[0]][pos[1]] = val

  fn print_grid() {
    set(robot, "@")
    print grid.map(fn (r) -> r.join("")).join("\n")
    set(robot, ".")
    print "\n"
  }

  fn build_stack(start, move) {
    const stack = []

    const Q = [start]
    while Q.len > 0 {
      let curr = Q.yank(0)
      curr = add(curr, DIRS[move])

      if curr in stack skip
      if at(curr)=="." skip // free
      if at(curr)=="#" ret [] // blocked
      if at(curr)=="[" {
        const other = add(curr, DIRS[">"])
        stack.push((curr, "["))
        stack.push((other, "]"))
        if (move == ">") Q.push(other)
        else if (move == "<") skip
        else {
          Q.push(curr)
          Q.push(other)
        }
      }
      if at(curr)=="]" {
        const other = add(curr, DIRS["<"])
        stack.push((curr, "]"))
        stack.push((other, "["))
        if (move == "<") Q.push(other)
        else if (move == ">") skip
        else {
          Q.push(curr)
          Q.push(other)
        }
      }
    }
  
    ret stack
  }

  for let i = 0; i<moves.len; i++; {
    const move = DIRS[moves[i]]
    const next = add(robot, move)

    if at(next)=="#" skip
    if at(next)=="O" {
      const stack = [next]
      let overnext = add(next, move)
      while at(overnext)=="O" {
        stack.push(overnext)
        overnext = add(overnext, move)
      }
      if at(overnext)=="." {
        // move robot and all boxes
        stack.each(fn (box)-> set(box, "."))
        stack.each(fn (box)-> set(add(box, move), "O"))
        robot = next
      }
      skip // box is blocked
    }
    if at(next) in "[]" {
      const stack = build_stack(robot, moves[i])
      if (stack.len > 0) {
        stack.each(fn (half) -> set(half[0], "."))
        stack.each(fn (half) -> set(add(half[0], move), half[1]))
        robot = next
      }
      skip // box is blocked
    }
    robot = next
  }

  const boxes = []
  const chr = bigboxes ? "[" : "O"
  grid.each(fn(row,y) {
    row.each(fn(_,x) -> at((y,x))==chr ? boxes.push((y,x)) : nil)
  })

  ret boxes.map(fn(box) -> box[0]*100+box[1]).sum()
}

log("Part 1:", simulate(false)) // Part 1: 1485257
log("Part 2:", simulate(true)) // Part 2: 