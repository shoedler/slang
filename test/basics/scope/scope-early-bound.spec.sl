let a = "outer"
{
  fn foo() {
    print a
  }

  foo() // [expect] outer
  let a = "inner"
  foo() // [expect] outer
}
