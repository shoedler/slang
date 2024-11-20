cls Foo {}

let foo = Foo()
foo() // [ExpectError] Uncaught error: Attempted to call non-callable value of type Foo.
      // [ExpectError]      4 | foo()
      // [ExpectError]             ~~
      // [ExpectError]   at line 4 at the toplevel of module "main"