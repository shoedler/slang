{
  cls Foo {
    fn returnSelf {
      ret Foo
    }
  }

  print Foo().returnSelf() // [Expect] <Class Foo>
}
