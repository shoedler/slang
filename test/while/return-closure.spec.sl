fn f() {
  while (true) {
    let i = "i"
    fn g() { print i }
    ret g;
  }
}

let h = f()
h() // [Expect] i
