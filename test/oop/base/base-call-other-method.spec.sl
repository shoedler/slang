cls Base {
  fn foo() {
    print "Base.foo()"
  }
}

cls Derived : Base {
  fn bar() {
    print "Derived.bar()"
    base.foo()
  }
}

Derived().bar()
// [expect] Derived.bar()
// [expect] Base.foo()
