{
  let a = "outer"
  {
    print a // [Expect] outer
    let a = "inner"
    print a // [Expect] inner
  }
}