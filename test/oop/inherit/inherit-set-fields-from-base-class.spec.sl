cls Foo {
  fn foo(a, b) {
    this.field1 = a
    this.field2 = b
  }

  fn foo_print() {
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
bar.foo_print()
// [expect] foo 1
// [expect] foo 2

bar.bar("bar 1", "bar 2")
bar.barPrint()
// [expect] bar 1
// [expect] bar 2

bar.foo_print()
// [expect] bar 1
// [expect] bar 2
