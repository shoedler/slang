cls Foo {}
Foo.bar // [ExpectError] Uncaught error: Property 'bar' does not exist on value of type Class.
        // [ExpectError]      2 | Foo.bar
        // [ExpectError]              ~~~
        // [ExpectError]   at line 2 at the toplevel of module "main"