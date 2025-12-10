import File
import Math

const ASCII_ZERO = 48

let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

let line_num = 0
for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  line_num = line_num + 1
  if line_num != 1 skip  // Only process first line
  
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
  
  print "n = " + n
  print "target m_num = " + m_num
  print "p configurations = " + p
  
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
  print "p_bits = " + p_bits
  
  let Q = [0]
  let explored = 0
  for let q_idx = 0; q_idx < Q.len; q_idx++; {
    let u = Q[q_idx]
    explored = explored + 1
    if explored % 100 == 0 {
      print "Explored " + explored + " states, queue size: " + Q.len
    }
    for let v_idx = 0; v_idx < p_bits.len; v_idx++; {
      let v = p_bits[v_idx]
      let new_state = Math.xor(u, v)
      
      if B[new_state] != -1 skip
      B[new_state] = B[u] + 1
      Q.push(new_state)
      
      if new_state == m_num {
        print "Found target! Distance: " + B[new_state]
      }
    }
  }
  
  print "Result for Part 1: " + B[m_num]
  print "Total states explored: " + explored
}
