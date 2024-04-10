fn inner_loop(outer_index) {
  for let j = 0; j < 3; j++; {
    if j == outer_index
      break
    print "  Inner loop: " + j.to_str()
  }
}

for let i = 0; i < 3; i++; {
  print "Outer loop: " + i.to_str()
  inner_loop(i)
}

// [Expect] Outer loop: 0
// [Expect] Outer loop: 1
// [Expect]   Inner loop: 0
// [Expect] Outer loop: 2
// [Expect]   Inner loop: 0
// [Expect]   Inner loop: 1