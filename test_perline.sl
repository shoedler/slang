import File
import Math

const INF = Float.inf
const EPS = 0.000000001
const ASCII_ZERO = 48

// Dummy simplex - returns all zeros
fn simplex(A, C) {
  let n = C.len
  let x = []
  for let i = 0; i < n; i++; {
    x.push(0)
  }
  ret [0, x]
}

// Simplified ILP
fn ilp(A) {
  let n = A[0].len - 1
  
  fn branch(A_current) {
    let c = []
    for let i = 0; i < n; i++; {
      c.push(1)
    }
    let val_x = simplex(A_current, c)
    let val = val_x[0]
    ret val
  }
  
  ret Math.floor(branch(A))
}

let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

let line_num = 0
for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  line_num = line_num + 1
  
  let parts = l.split(" ")
  let m = parts[0]
  let n = m.len - 2
  
  print "Line " + line_num + ": n=" + n + ", m=" + m
  
  // Quick ILP result
  let p_len = parts.len - 2
  let num_rows = 2 * n + p_len
  let num_cols = p_len + 1
  
  let A = []
  for let i = 0; i < num_rows; i++; {
    let row = []
    for let j = 0; j < num_cols; j++; {
      row.push(0)
    }
    A.push(row)
  }
  
  let p2_line = ilp(A)
  print "  Part2=" + p2_line
}
