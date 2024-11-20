{
  let a = "before"
  print a // [Expect] before

  a = "after"
  print a // [Expect] after

  print a = "arg" // [Expect] arg
  print a // [Expect] arg
}
