cls Skip {
  fn test(val) {
    let i = 0
    while i++ < 4 {
      if (i == 2) skip
      print i + val
    }
  }
}

for let i = 0; i < 3; i++; {
  if i == 1 skip
  print "Running Skip.test with i = " + i.to_str() + "..."
  Skip().test(i)
}

// [Expect] Running Skip.test with i = 0...
// [Expect] 1
// [Expect] 3
// [Expect] Running Skip.test with i = 2...
// [Expect] 3
// [Expect] 5