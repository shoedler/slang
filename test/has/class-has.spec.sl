cls Lol {
  fn x -> 1
  static fn y -> 2
}

print Lol.has("x") // [Expect] true
print Lol.has("y") // [Expect] true
print Lol.has("z") // [Expect] false

// Does not work with callables or any non-string type
print try Lol.has(fn (x) -> x == "x") else error // [Expect] Expected argument 0 of type Str but got Fn.
