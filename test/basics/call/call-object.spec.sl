cls Foo {}

let foo = Foo()
foo() // [expect-error] Uncaught error: Attempted to call non-callable value of type Foo.
      // [expect-error]      4 | foo()
      // [expect-error]             ~~
      // [expect-error]   at line 4 at the toplevel of module "main"