{
  let i = "before"

  // New variable is in inner scope.
  for let i = 0; i < 1; i++; {
    print i // [Expect] 0

    // Loop body is in second inner scope.
    let i = -1
    print i // [Expect] -1
  }
}

{
  // New variable shadows outer variable.
  for let i = 0; i > 0; i++; {}

  // Goes out of scope after loop.
  let i = "after"
  print i // [Expect] after

  // Can reuse an existing variable.
  for i = 0; i < 1; i++; {
    print i // [Expect] 0
  }
}
