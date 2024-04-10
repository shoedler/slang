fn inner_loop(outer_index) {
  let j = 0
  while j++ < 3 {
    if j == outer_index
      break
    print "  Inner loop: " + j.to_str()
  }
}

let i = 0
while i++ < 3 {
  print "Outer loop: " + i.to_str()
  inner_loop(i)
}

// [Expect] Outer loop: 1
// [Expect] Outer loop: 2
// [Expect]   Inner loop: 1