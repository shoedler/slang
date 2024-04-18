cls A {
  fn x -> 1
  static fn y -> 2
}

let a = A()

// Cannot access methods / static methods via get-indexing.
print a["x"]                // [Expect] nil
print try A["y"] else error // [Expect] Index 'y' does not exist on value of type Class.