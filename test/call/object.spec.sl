cls Foo {}

let foo = Foo()
foo() // [ExpectRuntimeError] Can only call functions and classes.
      // [ExpectRuntimeError] [line 4] in fn toplevel