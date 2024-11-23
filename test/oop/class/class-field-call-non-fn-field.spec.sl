cls Foo {}

let foo = Foo()
foo.bar = "not fn"

foo.bar() // [expect-error] Uncaught error: Attempted to call non-callable value of type Str.
          // [expect-error]      6 | foo.bar()
          // [expect-error]              ~~~~~
          // [expect-error]   at line 6 at the toplevel of module "main"