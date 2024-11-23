cls Foo {
  fn inFoo() {
    print "in foo"
  }
}

cls Bar : Foo {
  fn inBar() {
    print "in bar"
  }
}

cls Baz : Bar {
  fn inBaz() {
    print "in baz"
  }
}

let baz = Baz()
baz.inFoo() // [expect] in foo
baz.inBar() // [expect] in bar
baz.inBaz() // [expect] in baz
