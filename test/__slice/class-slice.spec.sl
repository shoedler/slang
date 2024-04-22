cls Lol {
  fn x -> "x"
  static fn y -> "y"
}

print Lol.slice(0, 0) // [ExpectRuntimeError] Uncaught error: Undefined method 'slice' in type Class or any of its parent classes.
                      // [ExpectRuntimeError]   at line 6 at the toplevel of module "main"

