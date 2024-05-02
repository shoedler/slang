let [a] = [a] // [ExpectError] Uncaught error: Undefined variable 'a'.
              // [ExpectError]      1 | let [a] = [a]
              // [ExpectError]                     ~
              // [ExpectError]   at line 1 at the toplevel of module "main"