(fn->1)[0] = 1 // [expect-error] Uncaught error: Type Fn does not support set-subscripting.
               // [expect-error]      1 | (fn->1)[0] = 1
               // [expect-error]                 ~~~~~~~
               // [expect-error]   at line 1 at the toplevel of module "main"