cls Lol {
  fn x -> "x"
  static fn y -> "y"
}

print Lol.slice(0, 0) // [expect-error] Uncaught error: Undefined callable 'slice' in type Class.
                      // [expect-error]      6 | print Lol.slice(0, 0)
                      // [expect-error]                    ~~~~~~~~~~~
                      // [expect-error]   at line 6 at the toplevel of module "main"