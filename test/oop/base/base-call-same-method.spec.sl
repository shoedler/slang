cls Base {
  fn foo() {
    print "Base.foo()"
  }
}

cls Derived : Base {
  fn foo() {
    print "Derived.foo()"
    base.foo()
  }
}

Derived().foo()
// [expect] Derived.foo()
// [expect] Base.foo()
