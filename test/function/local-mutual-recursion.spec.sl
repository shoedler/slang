{
  fn isEven(n) {
    if (n == 0) ret true;
    ret isOdd(n - 1); // [ExpectRuntimeError] Undefined variable 'isOdd'.
  }                   // [ExpectRuntimeError] [line 4] in fn "isEven"
                      // [ExpectRuntimeError] [line 12] in fn toplevel
  fn isOdd(n) {
    if (n == 0) ret false;
    ret isEven(n - 1);
  }

  isEven(4)
}