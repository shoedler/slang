// [exit] 3
true.foo // [expect-error] Uncaught error: Property 'foo' does not exist on value of type Bool.
         // [expect-error]      2 | true.foo
         // [expect-error]               ~~~
         // [expect-error]   at line 2 at the toplevel of module "main"