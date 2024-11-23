cls Foo{}
Foo[0] // [expect-error] Uncaught error: Type Class does not support get-subscripting.
       // [expect-error]      2 | Foo[0]
       // [expect-error]             ~~~
       // [expect-error]   at line 2 at the toplevel of module "main"