let a = "global"

{
  fn assign() {
    a = "assigned"
  }

  let a = "inner"
  assign()
  print a // [Expect] inner
}

print a // [Expect] assigned
