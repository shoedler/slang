let f

fn foo(param) {
  fn f_ { print param }
  f = f_
}

foo("param")
f() // [expect] param
