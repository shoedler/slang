cls A {
  fn foo() {
    print "A.foo()"
  }
}

cls B : A {}

cls C : B {
  fn foo() {
    print "C.foo()"
    base.foo()
  }
}

C().foo()
// [expect] C.foo()
// [expect] A.foo()
