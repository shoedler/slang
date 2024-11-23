let a = "1"
let a
print a
// [expect-error] Uncaught error: Variable 'a' is already defined.
// [expect-error]      2 | let a
// [expect-error]              ~
// [expect-error]   at line 2 at the toplevel of module "main"

// TODO: We should catch this at compile time.
