fn foo() {}

foo.bar // [ExpectError] Uncaught error: Property 'bar' does not exist on value of type Fn.
        // [ExpectError]      3 | foo.bar
        // [ExpectError]              ~~~
        // [ExpectError]   at line 3 at the toplevel of module "main"