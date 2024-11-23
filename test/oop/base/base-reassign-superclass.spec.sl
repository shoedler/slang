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
derived.method() // [expect] Base.method()
Base = OtherBase
derived.method() // [expect] Base.method()
