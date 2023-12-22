cls Foo {
  fn methodOnFoo() { print "foo" }
  fn override() { print "foo" }
}

cls Bar : Foo {
  fn methodOnBar() { print "bar" }
  fn override() { print "bar" }
}

let bar = Bar()
bar.methodOnFoo() // [Expect] foo
bar.methodOnBar() // [Expect] bar
bar.override() // [Expect] bar
