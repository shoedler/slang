// [exit] 3
import c // [expect-error] Parser error at line 1 at end: Expecting '}' after class body.
         // [expect-error]      1 | cls A {
         // [expect-error] 
         // [expect-error] Uncaught error: Running module 'c' failed with a compile error.
         // [expect-error]      2 | import c
         // [expect-error]          ~~~~~~~~
         // [expect-error]   at line 2 at the toplevel of module "main"