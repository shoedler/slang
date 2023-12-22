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
baz.inFoo() // [Expect] in foo
baz.inBar() // [Expect] in bar
baz.inBaz() // [Expect] in baz
