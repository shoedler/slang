// [exit] 3
print try try 1/nil else 1+nil else 1+nil // [expect-error] Uncaught error: Incompatible types for binary operand '+': Int + Nil.
                                          // [expect-error]      2 | print try try 1/nil else 1+nil else 1+nil
                                          // [expect-error]                                               ~~~~
                                          // [expect-error]   at line 2 at the toplevel of module "main"