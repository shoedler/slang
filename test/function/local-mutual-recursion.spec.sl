{
  fn isEven(n) {
    if (n == 0) ret true
    ret isOdd(n - 1) // [ExpectRuntimeError] Uncaught error: Undefined variable 'isOdd'.
  }                   // [ExpectRuntimeError] at line 4 in "isEven" in module "main"
                      // [ExpectRuntimeError] at line 12 at the toplevel of module "main"
  fn isOdd(n) {
    if (n == 0) ret false
    ret isEven(n - 1)
  }

  isEven(4)
}