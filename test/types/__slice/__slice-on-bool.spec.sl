// [exit] 3
print true.slice(0, 0) // [expect-error] Uncaught error: Undefined callable 'slice' in type Bool.
                       // [expect-error]      2 | print true.slice(0, 0)
                       // [expect-error]                     ~~~~~~~~~~~
                       // [expect-error]   at line 2 at the toplevel of module "main"