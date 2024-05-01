cls Foo {}
let foo = Foo()

foo.bar // [ExpectError] Uncaught error: Property 'bar' does not exist on value of type Foo.
        // [ExpectError]      4 | foo.bar
        // [ExpectError]              ~~~ (TODO: Manually added, fix this)
        // [ExpectError]   at line 4 at the toplevel of module "main"