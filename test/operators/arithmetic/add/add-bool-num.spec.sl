// [exit] 3
true + 123 // [expect-error] Uncaught error: Type Bool does not support the '+' operator. It must implement 'add'.
           // [expect-error]      2 | true + 123
           // [expect-error]               ~~~~~
           // [expect-error]   at line 2 at the toplevel of module "main"