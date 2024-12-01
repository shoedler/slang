// [exit] 3
"1" >= 1 // [expect-error] Uncaught error: Incompatible types for comparison operand >=. Left was Str, right was Int.
         // [expect-error]      2 | "1" >= 1
         // [expect-error]              ~~~~
         // [expect-error]   at line 2 at the toplevel of module "main"