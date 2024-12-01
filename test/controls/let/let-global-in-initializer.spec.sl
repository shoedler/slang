// [exit] 3
let a = "value"
let a = a  // [expect-error] Uncaught error: Variable 'a' is already defined.
print a    // [expect-error]      3 | let a = a
           // [expect-error]                  ~
           // [expect-error]   at line 3 at the toplevel of module "main"
