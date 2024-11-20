print {}.slice(0, 0) // [ExpectError] Uncaught error: Undefined callable 'slice' in type Obj.
                     // [ExpectError]      1 | print {}.slice(0, 0)
                     // [ExpectError]                   ~~~~~~~~~~~
                     // [ExpectError]   at line 1 at the toplevel of module "main"