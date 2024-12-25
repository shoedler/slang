fn inner_loop(outer_index) {
  let j = 0
  while ++j < 3 {
    if j == outer_index
      break
    print "  Inner loop: " + j
  }
}

let i = 0
while ++i < 3 {
  print "Outer loop: " + i
  inner_loop(i)
}

// [expect] Outer loop: 1
// [expect] Outer loop: 2
// [expect]   Inner loop: 1