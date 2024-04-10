let i = 0
while i++ < 3 {
  print "Outer loop: " + i.to_str()
  let j = 0
  while j++ < 3 {
    if j == i 
      skip // Skip the iteration where j equals i      
    print "  Inner loop: " + j.to_str()
  }
}

// [Expect] Outer loop: 1
// [Expect]   Inner loop: 2
// [Expect] Outer loop: 2
// [Expect]   Inner loop: 1