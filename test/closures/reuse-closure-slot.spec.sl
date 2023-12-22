{
  let f

  {
    let a = "a"
    fn f_ { print a }
    f = f_
  }

  {
    // Since a is out of scope, the local slot will be reused by b. Make sure
    // that f still closes over a.
    let b = "b"
    f() // [Expect] a
  }
}
