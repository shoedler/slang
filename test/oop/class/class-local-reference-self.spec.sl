{
  cls Foo {
    fn return_self {
      ret Foo
    }
  }

  print Foo().return_self() // [expect] <Class Foo>
}
