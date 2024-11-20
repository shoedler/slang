let i = 0
while i++ < 3 {
  print "Outer loop: " + i.to_str()
  let j = 0
  while j++ < 3 {
    if j == i 
      break  
    print "  Inner loop: " + j.to_str()
  }
}

// [Expect] Outer loop: 1
// [Expect] Outer loop: 2
// [Expect]   Inner loop: 1