cls Foo {
  ctor(a, b) {
    print "ctor" // [expect] ctor
    this.a = a
    this.b = b
  }
}

let foo = Foo(1, 2)
print foo.a // [expect] 1
print foo.b // [expect] 2
