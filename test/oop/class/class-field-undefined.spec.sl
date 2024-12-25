// [exit] 3
cls Foo {}
let foo = Foo()

foo.bar // [expect-error] Uncaught error: Property 'bar' does not exist on value of type Foo.
        // [expect-error]      5 | foo.bar
        // [expect-error]             ~~~~
        // [expect-error]   at line 5 at the toplevel of module "main"