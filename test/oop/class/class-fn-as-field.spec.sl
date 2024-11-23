// Without emitting OP_INVOKE
cls Foo {
  ctor {
    fn bar {
      print "not a method"
    }

    this.field = bar
  }
}

let foo = Foo()
foo.field() // [expect] not a method
print foo.field // [expect] <Fn bar>
