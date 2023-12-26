cls Foo {}

let foo = Foo()
foo() // [ExpectRuntimeError] Can only call functions and classes.
      // [ExpectRuntimeError] at line 4 at the toplevel