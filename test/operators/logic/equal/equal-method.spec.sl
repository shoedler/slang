// Bound methods have identity equality.
cls Foo {
  fn method() {}
}

let foo = Foo()
let foo_method = foo.method

// Same bound method.
print foo_method == foo_method // [expect] true

// Different closurizations.
print foo.method == foo.method // [expect] false
