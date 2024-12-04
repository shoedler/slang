// [exit] 3
print nil.slice(0, 0) // [expect-error] Uncaught error: Type Nil does not support "slice".
                      // [expect-error]      2 | print nil.slice(0, 0)
                      // [expect-error]                    ~~~~~~~~~~~
                      // [expect-error]   at line 2 at the toplevel of module "main"