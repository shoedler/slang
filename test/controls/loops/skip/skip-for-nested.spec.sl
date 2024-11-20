for let i = 0; i < 3; i++; {
  print "Outer loop: " + i.to_str()
  for let j = 0; j < 3; j++; {
    if j == i 
      skip // Skip the iteration where j equals i      
    print "  Inner loop: " + j.to_str()
  }
}

// [Expect] Outer loop: 0
// [Expect]   Inner loop: 1
// [Expect]   Inner loop: 2
// [Expect] Outer loop: 1
// [Expect]   Inner loop: 0
// [Expect]   Inner loop: 2
// [Expect] Outer loop: 2
// [Expect]   Inner loop: 0
// [Expect]   Inner loop: 1