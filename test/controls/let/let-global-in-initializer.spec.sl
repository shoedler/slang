let a = "value"
let a = a  // [expect-error] Uncaught error: Variable 'a' is already defined.
print a    // [expect-error]      2 | let a = a
           // [expect-error]                  ~
           // [expect-error]   at line 2 at the toplevel of module "main"
