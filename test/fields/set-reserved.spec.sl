cls X { ctor {} }

print X().__name = 123 // [ExpectRuntimeError] Uncaught error: Cannot set reserved property '__name' on value of type X.
                       // [ExpectRuntimeError]   at line 3 at the toplevel of module "main"