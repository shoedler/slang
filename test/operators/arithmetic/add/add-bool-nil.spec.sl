// [exit] 3
true + nil // [expect-error] Uncaught error: Incompatible types for binary operand +. Left was Bool, right was Nil.
           // [expect-error]      2 | true + nil
           // [expect-error]               ~~~~~
           // [expect-error]   at line 2 at the toplevel of module "main"