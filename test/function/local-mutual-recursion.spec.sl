{
  fn isEven(n) {
    if (n == 0) ret true;
    ret isOdd(n - 1); // [ExpectRuntimeError] Undefined variable 'isOdd'.
  }                   // [ExpectRuntimeError] at line 4 in "isEven"
                      // [ExpectRuntimeError] at line 12 at the toplevel
  fn isOdd(n) {
    if (n == 0) ret false;
    ret isEven(n - 1);
  }

  isEven(4)
}