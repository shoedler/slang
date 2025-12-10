import File
import Math

// Constants
const INF = Float.inf
const EPS = 0.000000001

// Simplex algorithm for linear programming
fn simplex(A, C) {
  let m = A.len
  let n = A[0].len - 1
  let N = []
  let B = []
  let D = []
  
  // Initialize N (non-basic variables)
  for let i = 0; i < n; i++; {
    N.push(i)
  }
  N.push(-1)
  
  // Initialize B (basic variables)
  for let i = n; i < n + m; i++; {
    B.push(i)
  }
  
  // Initialize D (tableau)
  for let i = 0; i < m; i++; {
    let row = []
    for let j = 0; j < n + 2; j++; {
      if j < n {
        row.push(A[i][j])
      } else if j == n {
        row.push(-1)
      } else {
        row.push(A[i][n])
      }
    }
    D.push(row)
  }
  
  // Add objective row
  let obj_row = []
  for let i = 0; i < n + 2; i++; {
    if i < n {
      obj_row.push(C[i])
    } else {
      obj_row.push(0)
    }
  }
  D.push(obj_row)
  
  // Add auxiliary row
  let aux_row = []
  for let i = 0; i < n + 2; i++; {
    aux_row.push(0)
  }
  D.push(aux_row)
  
  // Swap columns for right-hand side
  for let i = 0; i < m; i++; {
    let temp = D[i][n]
    D[i][n] = D[i][n + 1]
    D[i][n + 1] = temp
  }
  
  D[m + 1][n] = 1
  
  // Helper function: pivot operation
  fn pivot(r, s) {
    let k = 1.0 / D[r][s]
    for let i = 0; i < m + 2; i++; {
      if i == r skip
      for let j = 0; j < n + 2; j++; {
        if j != s {
          D[i][j] = D[i][j] - D[r][j] * D[i][s] * k
        }
      }
    }
    for let i = 0; i < n + 2; i++; {
      D[r][i] = D[r][i] * k
    }
    for let i = 0; i < m + 2; i++; {
      D[i][s] = D[i][s] * -k
    }
    D[r][s] = k
    let temp = B[r]
    B[r] = N[s]
    N[s] = temp
  }
  
  // Helper function: find entering and leaving variables
  fn find(p) {
    for ;;; {
      // Find entering variable (most negative reduced cost)
      let s = -1
      let min_val = 0
      let min_idx = -1
      
      for let i = 0; i <= n; i++; {
        if p == 0 and N[i] == -1 skip
        if D[m + p][i] < min_val or (D[m + p][i] == min_val and (min_idx == -1 or N[i] < N[min_idx])) {
          min_val = D[m + p][i]
          min_idx = i
          s = i
        }
      }
      
      if s == -1 or D[m + p][s] > -EPS ret 1
      
      // Find leaving variable (minimum ratio test)
      let r = -1
      let min_ratio = INF
      let min_b = -1
      
      for let i = 0; i < m; i++; {
        if D[i][s] > EPS {
          let ratio = D[i][n + 1] / D[i][s]
          if ratio < min_ratio or (ratio == min_ratio and (min_b == -1 or B[i] < min_b)) {
            min_ratio = ratio
            min_b = B[i]
            r = i
          }
        }
      }
      
      if r == -1 ret 0
      
      pivot(r, s)
    }
  }
  
  // Find initial basic feasible solution
  let r = 0
  for let i = 1; i < m; i++; {
    if D[i][n + 1] < D[r][n + 1] {
      r = i
    }
  }
  
  if D[r][n + 1] < -EPS {
    pivot(r, n)
    if !find(1) or D[m + 1][n + 1] < -EPS {
      ret [-INF, nil]
    }
  }
  
  // Remove auxiliary variables
  for let i = 0; i < m; i++; {
    if B[i] == -1 {
      let min_val = 0
      let min_idx = -1
      for let j = 0; j < n; j++; {
        if D[i][j] < min_val or (D[i][j] == min_val and (min_idx == -1 or N[j] < N[min_idx])) {
          min_val = D[i][j]
          min_idx = j
        }
      }
      if min_idx != -1 {
        pivot(i, min_idx)
      }
    }
  }
  
  // Solve the LP
  if find(0) {
    let x = []
    for let i = 0; i < n; i++; {
      x.push(0)
    }
    for let i = 0; i < m; i++; {
      if B[i] >= 0 and B[i] < n {
        x[B[i]] = D[i][n + 1]
      }
    }
    let val = 0
    for let i = 0; i < n; i++; {
      val = val + C[i] * x[i]
    }
    ret [val, x]
  } else {
    ret [-INF, nil]
  }
}

// Branch and bound ILP solver
fn ilp(A) {
  let n = A[0].len - 1
  let bval = INF
  let bsol = nil
  
  fn branch(A_current) {
    let val_x = simplex(A_current, (fn() {
      let c = []
      for let i = 0; i < n; i++; {
        c.push(1)
      }
      ret c
    })())
    let val = val_x[0]
    let x = val_x[1]
    
    if val + EPS >= bval or val == -INF {
      ret
    }
    
    // Find fractional variable
    let k = -1
    let v = 0
    if x != nil {
      for let i = 0; i < x.len; i++; {
        let e = x[i]
        if Math.abs(e - Math.floor(e + 0.5)) > EPS {
          k = i
          v = Math.floor(e)
          break
        }
      }
    }
    
    if k == -1 {
      // All variables are integer
      if val + EPS < bval {
        bval = val
        bsol = []
        for let i = 0; i < x.len; i++; {
          bsol.push(Math.floor(x[i] + 0.5))
        }
      }
    } else {
      // Branch on fractional variable
      let s1 = []
      for let i = 0; i < n; i++; {
        s1.push(0)
      }
      s1.push(v)
      s1[k] = 1
      
      let A1 = []
      for let i = 0; i < A_current.len; i++; {
        A1.push(A_current[i])
      }
      A1.push(s1)
      branch(A1)
      
      let s2 = []
      for let i = 0; i < n; i++; {
        s2.push(0)
      }
      s2.push(-(v + 1))
      s2[k] = -1
      
      let A2 = []
      for let i = 0; i < A_current.len; i++; {
        A2.push(A_current[i])
      }
      A2.push(s2)
      branch(A2)
    }
  }
  
  branch(A)
  if bval == INF {
    ret 0  // No solution found
  }
  ret Math.floor(bval + 0.5)
}

// Main program
let p1 = 0
let p2 = 0

let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  let parts = l.split(" ")
  let m = parts[0]
  let n = m.len - 2
  
  // Parse p values (button presses)
  let p = []
  for let i = 1; i < parts.len - 1; i++; {
    let coord_str = parts[i]
    // Remove trailing comma if present
    if coord_str[coord_str.len - 1] == "," {
      coord_str = coord_str[..-1]
    }
    // Parse tuple like "(0,1)"
    coord_str = coord_str[1..-1]  // Remove parens
    let coords = coord_str.split(",")
    let indices = []
    for let j = 0; j < coords.len; j++; {
      let num_str = coords[j]
      // Parse as int
      let num = 0
      let neg = false
      let start = 0
      if num_str.len > 0 and num_str[0] == "-" {
        neg = true
        start = 1
      }
      for let k = start; k < num_str.len; k++; {
        num = num * 10 + (num_str.ascii_at(k) - 48)
      }
      if neg num = -num
      indices.push(num)
    }
    p.push(indices)
  }
  
  // Parse c values (costs)
  let c_str = parts[parts.len - 1]
  c_str = c_str[1..-1]  // Remove brackets
  let c_parts = c_str.split(",")
  let c = []
  for let i = 0; i < c_parts.len; i++; {
    let num_str = c_parts[i]
    let num = 0
    let neg = false
    let start = 0
    if num_str.len > 0 and num_str[0] == "-" {
      neg = true
      start = 1
    }
    for let k = start; k < num_str.len; k++; {
      num = num * 10 + (num_str.ascii_at(k) - 48)
    }
    if neg num = -num
    c.push(num)
  }
  
  // Part 1: BFS with bitmask
  let B = []
  for let i = 0; i < Math.shl(1, n); i++; {
    B.push(-1)
  }
  B[0] = 0
  
  let p_bits = []
  for let i = 0; i < p.len; i++; {
    let bit_val = 0
    for let j = 0; j < p[i].len; j++; {
      bit_val = bit_val + Math.shl(1, p[i][j])
    }
    p_bits.push(bit_val)
  }
  
  let m_reversed = ""
  for let i = m.len - 2; i >= 1; i--; {
    let ch = m[i]
    if ch == "#" {
      m_reversed = m_reversed + "1"
    } else if ch == "." {
      m_reversed = m_reversed + "0"
    } else {
      m_reversed = m_reversed + ch
    }
  }
  
  // Convert binary string to number
  let m_num = 0
  for let i = 0; i < m_reversed.len; i++; {
    if m_reversed[i] == "1" {
      m_num = m_num + Math.shl(1, i)
    }
  }
  
  let Q = [0]
  for let q_idx = 0; q_idx < Q.len; q_idx++; {
    let u = Q[q_idx]
    for let v_idx = 0; v_idx < p_bits.len; v_idx++; {
      let v = p_bits[v_idx]
      let new_state = Math.xor(u, v)
      
      if B[new_state] != -1 skip
      B[new_state] = B[u] + 1
      Q.push(new_state)
    }
  }
  
  p1 = p1 + B[m_num]
  
  // Part 2: ILP
  let A = []
  let num_rows = 2 * n + p.len
  let num_cols = p.len + 1
  
  for let i = 0; i < num_rows; i++; {
    let row = []
    for let j = 0; j < num_cols; j++; {
      row.push(0)
    }
    A.push(row)
  }
  
  for let i = 0; i < p.len; i++; {
    // ~i in Python is -(i+1), which as a negative index means len-i-1
    let row_idx = num_rows - i - 1
    A[row_idx][i] = -1
    for let j = 0; j < p[i].len; j++; {
      let e = p[i][j]
      A[e][i] = 1
      A[e + n][i] = -1
    }
  }
  
  for let i = 0; i < n; i++; {
    A[i][num_cols - 1] = c[i]
    A[i + n][num_cols - 1] = -c[i]
  }
  
  p2 = p2 + ilp(A)
}

print "Part 1: " + p1
print "Part 2: " + p2
