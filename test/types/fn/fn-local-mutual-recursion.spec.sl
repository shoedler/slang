{
  fn isEven(n) {
    if (n == 0) ret true
    ret isOdd(n - 1)         // [expect-error] Uncaught error: Undefined variable 'isOdd'.
  }                          // [expect-error]      4 |     ret isOdd(n - 1)
                             // [expect-error]                  ~~~~~
  fn isOdd(n) {              // [expect-error]   at line 4 in "isEven" in module "main"
    if (n == 0) ret false    // [expect-error]   at line 12 at the toplevel of module "main"
    ret isEven(n - 1)
  }

  isEven(4)
}