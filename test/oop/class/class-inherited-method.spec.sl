cls Foo {
  fn in_foo() {
    print "in foo"
  }
}

cls Bar : Foo {
  fn in_bar() {
    print "in bar"
  }
}

cls Baz : Bar {
  fn in_baz() {
    print "in baz"
  }
}

let baz = Baz()
baz.in_foo() // [expect] in foo
baz.in_bar() // [expect] in bar
baz.in_baz() // [expect] in baz
