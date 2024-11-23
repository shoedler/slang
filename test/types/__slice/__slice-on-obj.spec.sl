print {}.slice(0, 0) // [expect-error] Uncaught error: Undefined callable 'slice' in type Obj.
                     // [expect-error]      1 | print {}.slice(0, 0)
                     // [expect-error]                   ~~~~~~~~~~~
                     // [expect-error]   at line 1 at the toplevel of module "main"