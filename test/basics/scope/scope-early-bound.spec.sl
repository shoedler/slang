let a = "outer"
{
  fn foo() {
    print a
  }

  foo() // [Expect] outer
  let a = "inner"
  foo() // [Expect] outer
}
