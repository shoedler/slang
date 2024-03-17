cls Foo {
  ctor {
    print "ctor"
    ret;
    print "nope"
  }
}

let foo = Foo()    // [Expect] ctor
print foo.__ctor() // [Expect] ctor
                   // [Expect] <Instance of Foo>
