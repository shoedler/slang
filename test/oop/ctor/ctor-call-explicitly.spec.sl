cls Foo {
  ctor(arg) {
    print "Foo.ctor(" + arg + ")"
    this.field = "ctor"
  }
}

let foo = Foo("one") // [expect] Foo.ctor(one)
foo.field = "field"

let foo2 = foo.ctor("two") // [expect] Foo.ctor(two)
print foo2                 // [expect] <Instance of Foo>

// Make sure ctor() doesn't create a fresh instance.
print foo.field // [expect] ctor
