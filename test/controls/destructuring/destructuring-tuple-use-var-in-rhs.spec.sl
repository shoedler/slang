// [exit] 3
let (a) = (a) // [expect-error] Uncaught error: Undefined variable 'a'.
              // [expect-error]      2 | let (a) = (a)
              // [expect-error]                     ~
              // [expect-error]   at line 2 at the toplevel of module "main"