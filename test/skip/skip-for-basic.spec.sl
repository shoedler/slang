for let i = 0; i < 10; i++; {
  if i % 2 == 0
    skip
  print i.to_str() + " is odd"
}

// [Expect] 1 is odd
// [Expect] 3 is odd
// [Expect] 5 is odd
// [Expect] 7 is odd
// [Expect] 9 is odd