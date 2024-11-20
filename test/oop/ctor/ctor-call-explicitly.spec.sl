cls Foo {
  ctor(arg) {
    print "Foo.ctor(" + arg + ")"
    this.field = "ctor"
  }
}

let foo = Foo("one") // [Expect] Foo.ctor(one)
foo.field = "field"

let foo2 = foo.ctor("two") // [Expect] Foo.ctor(two)
print foo2                 // [Expect] <Instance of Foo>

// Make sure ctor() doesn't create a fresh instance.
print foo.field // [Expect] ctor
