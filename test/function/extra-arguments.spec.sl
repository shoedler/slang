fn f(a, b) {
  print a
  print b
}

f(1, 2, 3, 4) // [ExpectError] Uncaught error: Expected 2 arguments but got 4.
              // [ExpectError]      6 | f(1, 2, 3, 4)
              // [ExpectError]           ~~~~~~~~~~~~
              // [ExpectError]   at line 6 at the toplevel of module "main"