cls Lol {
  fn x -> "x"
  static fn y -> "y"
}

print Lol.slice(0, 0) // [ExpectError] Uncaught error: Undefined method 'slice' in type Class.
                      // [ExpectError]      6 | print Lol.slice(0, 0)
                      // [ExpectError]                    ~~~~~~~~~~~
                      // [ExpectError]   at line 6 at the toplevel of module "main"