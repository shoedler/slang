cls Base {
  fn method() {
    print "Base.method()"
  }
}

cls Derived : Base {
  fn method() {
    base.method()
  }
}

cls OtherBase {
  fn method() {
    print "OtherBase.method()"
  }
}

let derived = Derived()
derived.method() // [Expect] Base.method()
Base = OtherBase
derived.method() // [Expect] Base.method()
