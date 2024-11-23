cls Foo {
  ctor {
    print "ctor"
    ret
    print "nope"
  }
}

let foo = Foo() // [expect] ctor
print foo // [expect] <Instance of Foo>
