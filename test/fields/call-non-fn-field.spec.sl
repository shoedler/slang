cls Foo {}

let foo = Foo()
foo.bar = "not fn"

foo.bar() // [ExpectError] Uncaught error: Attempted to call non-callable value of type Str.
          // [ExpectError]      6 | foo.bar()
          // [ExpectError]              ~~~~
          // [ExpectError]   at line 6 at the toplevel of module "main"