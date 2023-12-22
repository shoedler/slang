cls A {
  ctor(param) {
    this.field = param
  }

  fn test() {
    print this.field
  }
}

cls B : A {}

let b = B("value")
b.test() // [Expect] value
