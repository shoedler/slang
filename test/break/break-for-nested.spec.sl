for let i = 0; i < 4; i++; {
  print "Outer loop: " + i.to_str()
  for let j = 0; j < 3; j++; {
    if j == i 
      break
    print "  Inner loop: " + j.to_str()
  }

  if i == 2
    break
}

// [Expect] Outer loop: 0
// [Expect] Outer loop: 1
// [Expect]   Inner loop: 0
// [Expect] Outer loop: 2
// [Expect]   Inner loop: 0
// [Expect]   Inner loop: 1