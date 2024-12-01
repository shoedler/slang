// [exit] 3
cls Foo {}

let foo = Foo()
foo() // [expect-error] Uncaught error: Attempted to call non-callable value of type Foo.
      // [expect-error]      5 | foo()
      // [expect-error]             ~~
      // [expect-error]   at line 5 at the toplevel of module "main"