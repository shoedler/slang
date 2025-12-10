import File
import Math

const ASCII_ZERO = 48

let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

let p1_total = 0

for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  print "=== Line " + (line_idx + 1) + " ==="
  
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
  
  // Reverse and convert mask
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
  
  let m_num = 0
  for let i = 0; i < m_reversed.len; i++; {
    if m_reversed[i] == "1" {
      m_num = m_num + Math.shl(1, i)
    }
  }
  
  print "Target: " + m_num + " (binary: " + m_reversed + ")"
  
  // Part 1: BFS
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
  
  let result = B[m_num]
  print "BFS result: " + result
  p1_total = p1_total + result
}

print "==="
print "Part 1 total: " + p1_total
