{
  cls Foo {
    fn return_self {
      ret Foo
    }
  }

  print Foo().return_self() // [Expect] <Class Foo>
}