cls Foo {
  fn methodOnFoo() { print "foo" }
  fn override() { print "foo" }
}

cls Bar : Foo {
  fn methodOnBar() { print "bar" }
  fn override() { print "bar" }
}

let bar = Bar()
bar.methodOnFoo() // [expect] foo
bar.methodOnBar() // [expect] bar
bar.override() // [expect] bar
