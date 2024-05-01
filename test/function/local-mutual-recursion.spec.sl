{
  fn isEven(n) {
    if (n == 0) ret true
    ret isOdd(n - 1)         // [ExpectError] Uncaught error: Undefined variable 'isOdd'.
  }                          // [ExpectError]      4 |     ret isOdd(n - 1)
                             // [ExpectError]                  ~~~~~ (TODO: Manually added this. Fix it)
  fn isOdd(n) {              // [ExpectError]   at line 4 in "isEven" in module "main"
    if (n == 0) ret false    // [ExpectError]   at line 12 at the toplevel of module "main"
    ret isEven(n - 1)
  }

  isEven(4)
}