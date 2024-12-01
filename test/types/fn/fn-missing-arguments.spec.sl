// [exit] 3
fn f(a, b) {}

f(1) // [expect-error] Uncaught error: Expected 2 arguments but got 1.
     // [expect-error]      4 | f(1)
     // [expect-error]           ~~~
     // [expect-error]   at line 4 at the toplevel of module "main"