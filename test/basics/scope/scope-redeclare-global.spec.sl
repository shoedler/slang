let a = "1"
let a
print a
// [ExpectError] Uncaught error: Variable 'a' is already defined.
// [ExpectError]      2 | let a
// [ExpectError]              ~
// [ExpectError]   at line 2 at the toplevel of module "main"

// TODO: We should catch this at compile time.
