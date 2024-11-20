cls Foo {
  fn getClosure {
    fn f() {
      fn g() {
        fn h() -> this.toString()
        ret h
      }
      ret g
    }
    ret f
  }

  fn toString() { ret "Foo" }
}

let closure = Foo().getClosure()
print closure()()() // [Expect] Foo
