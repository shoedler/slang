import File
import Math

const INF = Float.inf
const EPS = 0.000000001
const ASCII_ZERO = 48

// Simplified simplex that just returns a dummy result for now
fn simplex(A, C) {
  print "  simplex called with A.len=" + A.len + ", C.len=" + C.len
  ret [10, [1, 1]]  // Dummy result
}

// ILP solver
fn ilp(A) {
  let n = A[0].len - 1
  print "ilp: n=" + n + ", A.len=" + A.len
  let bval = INF
  let bsol = nil
  let call_count = 0
  
  fn branch(A_current) {
    call_count = call_count + 1
    if call_count > 100 {
      print "  Too many calls, stopping"
      ret
    }
    
    print "  branch call " + call_count + ", A_current.len=" + A_current.len
    
    let val_x = simplex(A_current, (fn() {
      let c = []
      for let i = 0; i < n; i++; {
        c.push(1)
      }
      ret c
    })())
    let val = val_x[0]
    let x = val_x[1]
    
    print "    val=" + val + ", bval=" + bval
    
    if val + EPS >= bval or val == -INF {
      print "    pruned"
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
          print "    Found fractional var at k=" + k + ", value=" + e + ", v=" + v
          break
        }
      }
    }
    
    if k == -1 {
      print "    All integer, updating best"
      if val + EPS < bval {
        bval = val
        bsol = []
        for let i = 0; i < x.len; i++; {
          bsol.push(Math.floor(x[i] + 0.5))
        }
      }
    } else {
      print "    Branching on variable " + k
      // Branch
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
    ret 0
  }
  ret Math.floor(bval + 0.5)
}

// Test with first line
let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

let line_num = 0
for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  line_num = line_num + 1
  if line_num != 1 skip  // Only first line
  
  print "Processing line: " + l
  
  let parts = l.split(" ")
  let m = parts[0]
  let n = m.len - 2
  
  // Parse p values
  let p = []
  for let i = 1; i < parts.len - 1; i++; {
    let coord_str = parts[i]
    if coord_str[coord_str.len - 1] == "," {
      coord_str = coord_str[..-1]
    }
    coord_str = coord_str[1..-1]
    let coords = coord_str.split(",")
    let indices = []
    for let j = 0; j < coords.len; j++; {
      let num_str = coords[j]
      let num = 0
      let neg = false
      let start = 0
      if num_str.len > 0 and num_str[0] == "-" {
        neg = true
        start = 1
      }
      for let k = start; k < num_str.len; k++; {
        num = num * 10 + (num_str.ascii_at(k) - ASCII_ZERO)
      }
      if neg num = -num
      indices.push(num)
    }
    p.push(indices)
  }
  
  // Parse c values
  let c_str = parts[parts.len - 1]
  c_str = c_str[1..-1]
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
      num = num * 10 + (num_str.ascii_at(k) - ASCII_ZERO)
    }
    if neg num = -num
    c.push(num)
  }
  
  print "n=" + n + ", p.len=" + p.len + ", c=" + c
  
  // Part 2: ILP
  let num_rows = 2 * n + p.len
  let num_cols = p.len + 1
  
  print "Creating matrix: " + num_rows + "x" + num_cols
  
  let A = []
  for let i = 0; i < num_rows; i++; {
    let row = []
    for let j = 0; j < num_cols; j++; {
      row.push(0)
    }
    A.push(row)
  }
  
  for let i = 0; i < p.len; i++; {
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
  
  print "Calling ILP..."
  let result = ilp(A)
  print "ILP result: " + result
}
