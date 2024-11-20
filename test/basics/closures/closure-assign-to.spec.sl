let f
let g

{
  let local = "local"
  fn f_() {
    print local
    local = "after f"
    print local
  }
  f = f_

  fn g_() {
    print local
    local = "after g"
    print local
  }
  g = g_
}

f()
// [Expect] local
// [Expect] after f

g()
// [Expect] after f
// [Expect] after g
