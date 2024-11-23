for let i = 0; i < 10; i++; {
  if i % 2 == 0
    skip
  print i.to_str() + " is odd"
}

// [expect] 1 is odd
// [expect] 3 is odd
// [expect] 5 is odd
// [expect] 7 is odd
// [expect] 9 is odd