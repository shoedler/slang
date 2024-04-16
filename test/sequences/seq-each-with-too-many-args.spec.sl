print [1,2,3].each(fn (a,b,c) -> log(a,b,c)) // [ExpectRuntimeError] Uncaught error: Function passed to "each" must take 1 or 2 arguments, but got 3.
                                             // [ExpectRuntimeError]  at line 1 in native "each"
                                             // [ExpectRuntimeError]  at line 1 at the toplevel of module "main"