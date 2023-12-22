cls Foo {
  fn sayName(a) {
    print this.name
    print a
  }
}

let foo1 = Foo()
foo1.name = "foo1"

let foo2 = Foo()
foo2.name = "foo2"

// Store the method reference on another object.
foo2.fun = foo1.sayName
// Still retains original receiver.
foo2.fun(1)
// [Expect] foo1
// [Expect] 1
