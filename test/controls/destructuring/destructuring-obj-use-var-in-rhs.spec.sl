let {a} = {a:1} // [expect-error] Uncaught error: Undefined variable 'a'.
                // [expect-error]      1 | let {a} = {a:1}
                // [expect-error]                     ~
                // [expect-error]   at line 1 at the toplevel of module "main"