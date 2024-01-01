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
// [Expect] Derived.foo()
// [Expect] Base.foo()
