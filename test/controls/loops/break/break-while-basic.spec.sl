let i = 0
while i++ < 10 {
  if i % 2 == 0
    break
  print i.to_str() + " is odd"
}

// [Expect] 1 is odd
