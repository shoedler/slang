fn f(a, b) {
  print a
  print b
}

f(1, 2, 3, 4) // [ExpectRuntimeError] Expected 2 arguments but got 4.
              // [ExpectRuntimeError] at line 6 at the toplevel