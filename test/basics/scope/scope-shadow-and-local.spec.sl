{
  let a = "outer"
  {
    print a // [expect] outer
    let a = "inner"
    print a // [expect] inner
  }
}