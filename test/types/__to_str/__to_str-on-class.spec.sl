cls Lol {
  fn x -> "x"
  static fn y -> "y"
}

print Lol.to_str() // [expect] <Class Lol>

print Lol // [expect] <Class Lol>