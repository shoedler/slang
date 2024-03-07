cls Foo {}

let foo = Foo()
foo() // [ExpectRuntimeError] Attempted to call non-callable value of type Instance.
      // [ExpectRuntimeError] at line 4 at the toplevel