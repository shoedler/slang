// [exit] 3
{
  fn is_even(n) {
    if (n == 0) ret true
    ret is_odd(n - 1)         // [expect-error] Uncaught error: Undefined variable 'is_odd'.
  }                          // [expect-error]      5 |     ret is_odd(n - 1)
                             // [expect-error]                  ~~~~~~
  fn is_odd(n) {              // [expect-error]   at line 5 in "is_even" in module "main"
    if (n == 0) ret false    // [expect-error]   at line 13 at the toplevel of module "main"
    ret is_even(n - 1)
  }

  is_even(4)
}