// Bound methods have identity equality.
cls Foo {
  fn method() {}
}

let foo = Foo()
let fooMethod = foo.method

// Same bound method.
print fooMethod == fooMethod // [expect] true

// Different closurizations.
print foo.method == foo.method // [expect] false
