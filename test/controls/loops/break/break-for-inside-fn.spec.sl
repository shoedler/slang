fn inner_loop(outer_index) {
  for let j = 0; j < 3; j++; {
    if j == outer_index
      break
    print "  Inner loop: " + j
  }
}

for let i = 0; i < 3; i++; {
  print "Outer loop: " + i
  inner_loop(i)
}

// [expect] Outer loop: 0
// [expect] Outer loop: 1
// [expect]   Inner loop: 0
// [expect] Outer loop: 2
// [expect]   Inner loop: 0
// [expect]   Inner loop: 1