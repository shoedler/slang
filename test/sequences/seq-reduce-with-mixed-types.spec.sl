// Passing an anonymous function with index
print [1,3,4].reduce("", fn(acc, x) -> acc + x) // [ExpectRuntimeError] Uncaught error: Incompatible types for binary operand +. Left was Str, right was Int.
                                                // [ExpectRuntimeError]   at line 2 in "(anon)" in module "main"
                                                // [ExpectRuntimeError]   at line 2 at the toplevel of module "main"