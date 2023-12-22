let f

{
  let local = "local"
  fn f_ { print local }
  f = f_
}

f() // [Expect] local
