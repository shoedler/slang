{
  let a = "local"
  {
    let a = "shadow"
    print a // [Expect] shadow
  }
  print a // [Expect] local
}
