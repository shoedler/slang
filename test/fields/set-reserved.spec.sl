cls X { ctor {} }

print X().__name = 123 // [ExpectError] Uncaught error: Cannot set reserved property '__name' on value of type X.
                       // [ExpectError]      3 | print X().__name = 123
                       // [ExpectError]                    ~~~~~~~~~~~~
                       // [ExpectError]   at line 3 at the toplevel of module "main"