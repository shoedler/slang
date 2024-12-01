// [exit] 3
true + "s" // [expect-error] Uncaught error: Incompatible types for binary operand +. Left was Bool, right was Str.
           // [expect-error]      2 | true + "s"
           // [expect-error]               ~~~~~
           // [expect-error]   at line 2 at the toplevel of module "main"