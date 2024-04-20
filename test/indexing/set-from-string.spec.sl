print "foo"[0] = "a" // [ExpectRuntimeError] Uncaught error: Type Str does not support set-indexing with Num.
                     // [ExpectRuntimeError] at line 1 at the toplevel of module "main"