// [exit] 2
{
  fn is_even(n) {
    if (n == 0) ret true
    ret is_odd(n - 1) // [expect-error] Resolver error at line 5: Undefined variable 'is_odd'.
  }                   // [expect-error]      5 |     ret is_odd(n - 1)
                      // [expect-error]                  ~~~~~~
  fn is_odd(n) {
    if (n == 0) ret false
    ret is_even(n - 1)
  }

  is_even(4)
}