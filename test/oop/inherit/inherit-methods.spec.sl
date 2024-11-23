cls Foo {
  fn method_on_foo() { print "foo" }
  fn override() { print "foo" }
}

cls Bar : Foo {
  fn method_on_bar() { print "bar" }
  fn override() { print "bar" }
}

let bar = Bar()
bar.method_on_foo() // [expect] foo
bar.method_on_bar() // [expect] bar
bar.override() // [expect] bar
