cls X { ctor {} }

print X().__name = 123 // [expect-error] Uncaught error: Cannot set reserved property '__name' on value of type X.
                       // [expect-error]      3 | print X().__name = 123
                       // [expect-error]                    ~~~~~~~~~~~~
                       // [expect-error]   at line 3 at the toplevel of module "main"