cls Foo {}

let foo = Foo()
foo.bar = "not fn"

foo.bar() // [ExpectRuntimeError] Can only call functions and classes.
          // [ExpectRuntimeError] [line 6] in fn toplevel