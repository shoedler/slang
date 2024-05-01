fn f(a, b) {}

f(1) // [ExpectError] Uncaught error: Expected 2 arguments but got 1.
     // [ExpectError]      3 | f(1)
     // [ExpectError]           ~~
     // [ExpectError]   at line 3 at the toplevel of module "main"