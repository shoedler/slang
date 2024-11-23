cls Foo {
  fn get_closure() {
    fn closure() {
      ret this.get_foo()
    }
    ret closure
  }

  fn get_foo() { ret "Foo" }
}

let closure = Foo().get_closure()
print closure() // [expect] Foo
