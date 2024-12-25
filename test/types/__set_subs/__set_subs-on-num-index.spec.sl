// [exit] 3
123[1] = 1 // [expect-error] Uncaught error: Type Int does not support set-subscripting.
           // [expect-error]      2 | 123[1] = 1
           // [expect-error]             ~~~
           // [expect-error]   at line 2 at the toplevel of module "main"