// [exit] 3
undefined1.bar // [expect-error] Uncaught error: Undefined variable 'undefined1'.
  = undefined2 // [expect-error]      2 | undefined1.bar
               // [expect-error]          ~~~~~~~~~~
               // [expect-error]   at line 2 at the toplevel of module "main"