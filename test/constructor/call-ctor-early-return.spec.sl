cls Foo {
  ctor {
    print "ctor"
    ret;
    print "nope"
  }
}

let foo = Foo()  // [Expect] ctor
print foo.ctor() // [Expect] ctor
                 // [Expect] <Instance of Foo>
