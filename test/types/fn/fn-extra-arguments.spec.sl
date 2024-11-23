fn f(a, b) {
  print a
  print b
}

f(1, 2, 3, 4) // [expect-error] Uncaught error: Expected 2 arguments but got 4.
              // [expect-error]      6 | f(1, 2, 3, 4)
              // [expect-error]           ~~~~~~~~~~~~
              // [expect-error]   at line 6 at the toplevel of module "main"