cls Foo {
  fn bar(arg) {
    print arg
  }
}

let bar = Foo().bar
print "got method" // [expect] got method
bar("arg")         // [expect] arg
