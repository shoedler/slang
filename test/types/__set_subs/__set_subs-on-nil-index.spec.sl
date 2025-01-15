// [exit] 3
nil[0] = 1 // [expect-error] Uncaught error: Type Nil does not support set-subscripting.
           // [expect-error]      2 | nil[0] = 1
           // [expect-error]             ~~~
           // [expect-error]   at line 2 at the toplevel of module "main"