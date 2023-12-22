cls Foo {
  fn getClosure() {
    fn closure() {
      ret this.toString();
    }
    ret closure;
  }

  fn toString() { ret "Foo"; }
}

let closure = Foo().getClosure()
print closure() // [Expect] Foo
