// [exit] 3
(fn -> 1)[0] // [expect-error] Uncaught error: Type Fn does not support get-subscripting.
             // [expect-error]      2 | (fn -> 1)[0]
             // [expect-error]                   ~~~
             // [expect-error]   at line 2 at the toplevel of module "main"