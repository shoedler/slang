// [exit] 3
cls Lol {
  fn x -> "x"
  static fn y -> "y"
}

print Lol.slice(0, 0) // [expect-error] Uncaught error: Type Class does not support "slice".
                      // [expect-error]      7 | print Lol.slice(0, 0)
                      // [expect-error]                   ~~~~~~~~~~~~
                      // [expect-error]   at line 7 at the toplevel of module "main"