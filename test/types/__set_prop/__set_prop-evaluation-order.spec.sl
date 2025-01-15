// [exit] 2
undefined1.bar = undefined2 // [expect-error] Resolver error at line 2: Undefined variable 'undefined2'.
                            // [expect-error]      2 | undefined1.bar = undefined2
                            // [expect-error]                           ~~~~~~~~~~
                            // [expect-error] Resolver error at line 2: Undefined variable 'undefined1'.
                            // [expect-error]      2 | undefined1.bar = undefined2
                            // [expect-error]          ~~~~~~~~~~