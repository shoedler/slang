cls Foo {}
let foo = Foo()

foo.bar // [ExpectRuntimeError] Uncaught error: Property 'bar' does not exist on value of type Foo.
        // [ExpectRuntimeError] at line 4 at the toplevel of module "main"
