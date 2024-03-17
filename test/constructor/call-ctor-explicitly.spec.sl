cls Foo {
  ctor(arg) {
    print "Foo.__ctor(" + arg + ")"
    this.field = "ctor"
  }
}

let foo = Foo("one") // [Expect] Foo.__ctor(one)
foo.field = "field"

let foo2 = foo.__ctor("two") // [Expect] Foo.__ctor(two)
print foo2                   // [Expect] <Instance of Foo>

// Make sure ctor() doesn't create a fresh instance.
print foo.field // [Expect] ctor
