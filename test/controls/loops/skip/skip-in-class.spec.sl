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
  print "Running Skip.test with i = " + i + "..."
  Skip().test(i)
}

// [expect] Running Skip.test with i = 0...
// [expect] 1
// [expect] 3
// [expect] Running Skip.test with i = 2...
// [expect] 3
// [expect] 5