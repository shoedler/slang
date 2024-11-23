fn f() {
  for ;;; {
    let i = "i"
    fn g() { print i }
    ret g
  }
}

let h = f()
h() // [expect] i
