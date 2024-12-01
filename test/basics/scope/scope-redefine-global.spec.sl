// [exit] 3
let a = "1"
let a = "2"
print a
 // [expect-error] Uncaught error: Variable 'a' is already defined.
 // [expect-error]      3 | let a = "2"
 // [expect-error]                  ~~~
 // [expect-error]   at line 3 at the toplevel of module "main"

// TODO: We should catch this at compile time.
