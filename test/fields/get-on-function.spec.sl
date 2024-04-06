fn foo() {}

foo.bar // [ExpectRuntimeError] Uncaught error: Property 'bar' does not exist on type Fn.
        // [ExpectRuntimeError] at line 3 at the toplevel of module "main"