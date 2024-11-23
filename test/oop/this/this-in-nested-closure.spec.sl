cls Foo {
  fn get_closure {
    fn f() {
      fn g() {
        fn h() -> this.get_foo()
        ret h
      }
      ret g
    }
    ret f
  }

  fn get_foo() { ret "Foo" }
}

let closure = Foo().get_closure()
print closure()()() // [expect] Foo
