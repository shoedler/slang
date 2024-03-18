cls Foo {
  ctor {
    print "ctor"
    ret;
    print "nope"
  }
}

let foo = Foo() // [Expect] ctor
print foo // [Expect] <Instance of Foo>
