cls X { ctor {} }

print X().__name = 123 // [ExpectRuntimeError] Cannot set reserved field '__name'.
                       // [ExpectRuntimeError]   at line 3 at the toplevel of module "main"