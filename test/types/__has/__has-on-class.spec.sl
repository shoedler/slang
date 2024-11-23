cls Lol {
  fn x -> 1
  static fn y -> 2
}

// Instance methods are not 'has'able
print Lol.has("x") // [expect] false
print Lol.has("y") // [expect] true
print Lol.has("z") // [expect] false

// Does not work with callables or any non-string type
print try Lol.has(fn (x) -> x == "x") else error // [expect] Expected argument 0 of type Str but got Fn.
