// [exit] 3
undefined1.bar // [expect-error] Resolver error at line 2: Undefined variable 'undefined1'.
  = undefined2 // [expect-error]      2 | undefined1.bar
               // [expect-error]          ~~~~~~~~~~
               // [expect-error] Resolver error at line 3: Undefined variable 'undefined2'.
               // [expect-error]      3 |   = undefined2
               // [expect-error]              ~~~~~~~~~~