cls Foo {
  fn return_self {
    ret Foo
  }
}

print Foo().return_self() // [expect] <Foo>
print Foo().return_self()() // [expect] <Instance of Foo>
