// [exit] 3
print {}.slice(0, 0) // [expect-error] Uncaught error: Type Obj does not support "slice".
                     // [expect-error]      2 | print {}.slice(0, 0)
                     // [expect-error]                   ~~~~~~~~~~~
                     // [expect-error]   at line 2 at the toplevel of module "main"