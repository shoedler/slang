cls Base {
  fn toString() { ret "Base"; }
}

cls Derived : Base {
  fn getClosure() {
    fn closure() {
      ret base.toString();
    }
    ret closure;
  }

  fn toString -> "Derived"
}

let closure = Derived().getClosure()
print closure() // [Expect] Base
