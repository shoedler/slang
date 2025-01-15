let i = 0
while ++i < 3 {
  print "Outer loop: " + i
  let j = 0
  while ++j < 3 {
    if j == i 
      break  
    print "  Inner loop: " + j
  }
}

// [expect] Outer loop: 1
// [expect] Outer loop: 2
// [expect]   Inner loop: 1