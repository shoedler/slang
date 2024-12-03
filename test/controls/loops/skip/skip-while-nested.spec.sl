let i = 0
while i++ < 3 {
  print "Outer loop: " + i
  let j = 0
  while j++ < 3 {
    if j == i 
      skip // Skip the iteration where j equals i      
    print "  Inner loop: " + j
  }
}

// [expect] Outer loop: 1
// [expect]   Inner loop: 2
// [expect] Outer loop: 2
// [expect]   Inner loop: 1