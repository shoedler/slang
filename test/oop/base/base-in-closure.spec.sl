cls Base {
  fn get_name() { ret Base.__name }
}

cls Derived : Base {
  fn getClosure() {
    fn closure() {
      ret base.get_name()
    }
    ret closure
  }

  fn get_name -> "Derived"
}

let closure = Derived().getClosure()
print closure() // [expect] Base
