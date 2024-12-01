// [exit] 3
cls Foo {}

let foo = Foo()
foo.bar = "not fn"

foo.bar() // [expect-error] Uncaught error: Attempted to call non-callable value of type Str.
          // [expect-error]      7 | foo.bar()
          // [expect-error]              ~~~~~
          // [expect-error]   at line 7 at the toplevel of module "main"