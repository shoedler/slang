fn foo() {}

foo.bar // [expect-error] Uncaught error: Property 'bar' does not exist on value of type Fn.
        // [expect-error]      3 | foo.bar
        // [expect-error]              ~~~
        // [expect-error]   at line 3 at the toplevel of module "main"