undefined1.bar // [ExpectError] Uncaught error: Undefined variable 'undefined1'.
  = undefined2 // [ExpectError]      1 | undefined1.bar
               // [ExpectError]                     ~~~ (TODO: Manually added, fix this)
               // [ExpectError]   at line 1 at the toplevel of module "main"