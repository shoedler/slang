// [exit] 3
true + "s" // [expect-error] Uncaught error: Type Bool does not support "add".
           // [expect-error]      2 | true + "s"
           // [expect-error]               ~~~~~
           // [expect-error]   at line 2 at the toplevel of module "main"