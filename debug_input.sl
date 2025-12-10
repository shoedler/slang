import File
import Math

const ASCII_ZERO = 48

let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  print "Line: " + l
  
  let parts = l.split(" ")
  print "Parts: " + parts
  
  let m = parts[0]
  print "m: " + m
  let n = m.len - 2
  print "n: " + n
  
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
  print "p: " + p
  
  // Parse c values
  let c_str = parts[parts.len - 1]
  print "c_str before: " + c_str
  c_str = c_str[1..-1]
  print "c_str after: " + c_str
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
  print "c: " + c
  
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
  print "m_reversed: " + m_reversed
  
  let m_num = 0
  for let i = 0; i < m_reversed.len; i++; {
    if m_reversed[i] == "1" {
      m_num = m_num + Math.shl(1, i)
    }
  }
  print "m_num: " + m_num
  
  print "---"
}
