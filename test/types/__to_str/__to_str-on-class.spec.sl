cls Lol {
  fn x -> "x"
  static fn y -> "y"
}

print Lol.to_str() // [expect] <Lol>

print Lol // [expect] <Lol>