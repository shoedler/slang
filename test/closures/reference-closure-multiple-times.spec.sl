let f

{
  let a = "a"
  fn f_ {
    print a
    print a
  }
  f = f_
}

f()
// [Expect] a
// [Expect] a
