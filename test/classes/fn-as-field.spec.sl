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
foo.field() // [Expect] not a method
print foo.field // [Expect] [Fn bar, arity 0]
