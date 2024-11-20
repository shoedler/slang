cls Foo {
  fn foo(a, b) {
    this.field1 = a
    this.field2 = b
  }

  fn fooPrint() {
    print this.field1
    print this.field2
  }
}

cls Bar : Foo {
  fn bar(a, b) {
    this.field1 = a
    this.field2 = b
  }

  fn barPrint() {
    print this.field1
    print this.field2
  }
}

let bar = Bar()
bar.foo("foo 1", "foo 2")
bar.fooPrint()
// [Expect] foo 1
// [Expect] foo 2

bar.bar("bar 1", "bar 2")
bar.barPrint()
// [Expect] bar 1
// [Expect] bar 2

bar.fooPrint()
// [Expect] bar 1
// [Expect] bar 2
