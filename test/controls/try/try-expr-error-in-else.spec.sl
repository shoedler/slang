// [exit] 3
print try try ASLDJKASLDJK else 1+nil else 1+nil // [expect-error] Uncaught error: Incompatible types for binary operand +. Left was Int, right was Nil.
                                                 // [expect-error]      2 | print try try ASLDJKASLDJK else 1+nil else 1+nil
                                                 // [expect-error]                                                      ~~~~
                                                 // [expect-error]   at line 2 at the toplevel of module "main"