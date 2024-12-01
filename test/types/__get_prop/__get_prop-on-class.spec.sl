// [exit] 3
cls Foo {}
Foo.bar // [expect-error] Uncaught error: Property 'bar' does not exist on value of type Class.
        // [expect-error]      3 | Foo.bar
        // [expect-error]              ~~~
        // [expect-error]   at line 3 at the toplevel of module "main"