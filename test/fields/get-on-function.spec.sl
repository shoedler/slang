fn foo() {}

foo.bar // [ExpectRuntimeError] Property 'bar' does not exist on type Fn.
        // [ExpectRuntimeError] at line 3 at the toplevel of module "main"