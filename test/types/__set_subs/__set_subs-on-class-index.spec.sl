// [exit] 3
cls Foo{}
Foo[0] = 1 // [expect-error] Uncaught error: Type Class does not support set-subscripting.
           // [expect-error]      3 | Foo[0] = 1
           // [expect-error]             ~~~
           // [expect-error]   at line 3 at the toplevel of module "main"