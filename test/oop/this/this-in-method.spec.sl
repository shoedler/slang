cls Foo {
  fn bar -> this
  fn baz -> "baz"
}

print Foo().bar().baz() // [expect] baz
