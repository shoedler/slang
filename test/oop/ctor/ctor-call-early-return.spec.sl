cls Foo {
  ctor {
    print "ctor"
    ret
    print "nope"
  }
}

let foo = Foo()  // [expect] ctor
print foo.ctor() // [expect] ctor
                 // [expect] <Instance of Foo>
