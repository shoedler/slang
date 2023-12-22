cls Foo {
  ctor(a, b) {
    print "ctor" // [Expect] ctor
    this.a = a
    this.b = b
  }
}

let foo = Foo(1, 2)
print foo.a // [Expect] 1
print foo.b // [Expect] 2
