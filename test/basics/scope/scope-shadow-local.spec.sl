{
  let a = "local"
  {
    let a = "shadow"
    print a // [expect] shadow
  }
  print a // [expect] local
}
