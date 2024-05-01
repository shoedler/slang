// Passing an anonymous function with index
print [1,3,4].reduce("", fn(acc, x) -> acc + x) // [ExpectError] Uncaught error: Incompatible types for binary operand +. Left was Str, right was Int.
                                                // [ExpectError]      2 | print [1,3,4].reduce("", fn(acc, x) -> acc + x)
                                                // [ExpectError]                                                     ~~
                                                // [ExpectError]   at line 2 in "(anon)" in module "main"
                                                // [ExpectError]   at line 2 at the toplevel of module "main"