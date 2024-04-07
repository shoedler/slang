cls Foo {}

let foo = Foo()
foo() // [ExpectRuntimeError] Uncaught error: Attempted to call non-callable value of type Foo.
      // [ExpectRuntimeError] at line 4 at the toplevel of module "main"