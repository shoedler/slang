let a = "outer"

{
  let a = "inner"
  print a // [Expect] inner
}

print a // [Expect] outer
