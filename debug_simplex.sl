import File

let input = File.read(File.join_path(cwd(), "input.txt"))
let lines = input.split("\n")

for let line_idx = 0; line_idx < lines.len; line_idx++; {
  let l = lines[line_idx]
  if l.len == 0 skip
  
  let parts = l.split(" ")
  print "Parts: " + parts
  let m = parts[0]
  let n = m.len - 2
  
  print "m: " + m
  print "n: " + n
  
  // Parse p values (button presses)
  let p = []
  for let i = 1; i < parts.len - 1; i++; {
    let coord_str = parts[i]
    print "coord_str: " + coord_str
    // Remove trailing comma if present
    if coord_str[coord_str.len - 1] == "," {
      coord_str = coord_str[..-1]
    }
    // Parse tuple like "(0,1)"
    coord_str = coord_str[1..-1]  // Remove parens
    let coords = coord_str.split(",")
    print "coords: " + coords
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
    print "indices: " + indices
    p.push(indices)
  }
  
  print "p: " + p
  print "p.len: " + p.len
  
  // Parse c values (costs)
  let c_str = parts[parts.len - 1]
  print "c_str: " + c_str
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
  
  print "c: " + c
  print "c.len: " + c.len
  
  // Part 2: ILP
  let num_rows = 2 * n + p.len
  let num_cols = p.len + 1
  
  print "num_rows: " + num_rows
  print "num_cols: " + num_cols
  
  for let i = 0; i < p.len; i++; {
    // ~i in Python is -(i+1), which as a negative index means len-i-1
    let row_idx = num_rows - i - 1
    print "i: " + i + ", row_idx: " + row_idx
    for let j = 0; j < p[i].len; j++; {
      let e = p[i][j]
      print "  e: " + e + " (should be < " + num_rows + ")"
    }
  }
}
