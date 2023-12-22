cls Foo {
  fn returnSelf {
    ret Foo;
  }
}

print Foo().returnSelf() // [Expect] Foo
print Foo().returnSelf()() // [Expect] [Instance of Foo]
