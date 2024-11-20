let a = "value"
let a = a  // [ExpectError] Uncaught error: Variable 'a' is already defined.
print a    // [ExpectError]      2 | let a = a
           // [ExpectError]                  ~
           // [ExpectError]   at line 2 at the toplevel of module "main"
