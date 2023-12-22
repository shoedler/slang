cls Foo {
  fn bar(arg) {
    print arg
  }
}

let bar = Foo().bar
print "got method" // [Expect] got method
bar("arg")         // [Expect] arg
