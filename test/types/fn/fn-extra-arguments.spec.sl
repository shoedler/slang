// [exit] 3
fn f(a, b) {
  print a
  print b
}

f(1, 2, 3, 4) // [expect-error] Uncaught error: Expected 2 arguments but got 4.
              // [expect-error]      7 | f(1, 2, 3, 4)
              // [expect-error]           ~~~~~~~~~~~~
              // [expect-error]   at line 7 at the toplevel of module "main"