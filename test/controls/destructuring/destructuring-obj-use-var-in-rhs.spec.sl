// [exit] 3
let {a} = {a:1} // [expect-error] Uncaught error: Undefined variable 'a'.
                // [expect-error]      2 | let {a} = {a:1}
                // [expect-error]                     ~
                // [expect-error]   at line 2 at the toplevel of module "main"