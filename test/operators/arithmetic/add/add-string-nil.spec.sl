// [exit] 3
"s" + nil // [expect-error] Uncaught error: Incompatible types for binary operand +. Left was Str, right was Nil.
          // [expect-error]      2 | "s" + nil
          // [expect-error]              ~~~~~
          // [expect-error]   at line 2 at the toplevel of module "main"