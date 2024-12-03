for let i = 0; i < 4; i++; {
  print "Outer loop: " + i
  for let j = 0; j < 3; j++; {
    if j == i 
      break
    print "  Inner loop: " + j
  }

  if i == 2
    break
}

// [expect] Outer loop: 0
// [expect] Outer loop: 1
// [expect]   Inner loop: 0
// [expect] Outer loop: 2
// [expect]   Inner loop: 0
// [expect]   Inner loop: 1