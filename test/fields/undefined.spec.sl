cls Foo {}
let foo = Foo()

foo.bar // [ExpectRuntimeError] Uncaught error: Property 'bar' does not exist on type Instance.
        // [ExpectRuntimeError] at line 4 at the toplevel of module "main"
