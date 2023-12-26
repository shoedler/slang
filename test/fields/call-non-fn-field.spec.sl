cls Foo {}

let foo = Foo()
foo.bar = "not fn"

foo.bar() // [ExpectRuntimeError] Can only call functions and classes.
          // [ExpectRuntimeError] at line 6 at the toplevel