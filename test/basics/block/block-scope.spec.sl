let a = "outer"

{
  let a = "inner"
  print a // [expect] inner
}

print a // [expect] outer
