cls Lol {
  ctor { this.y = 1 }
  fn x -> this.y
  static fn y -> 2
}

let new = {}
let construct = Lol.ctor.bind(new)

print construct() // [Expect] {y: 1}
print typeof(new) // [Expect] <Class Obj>
print try new.x() else error // [Expect] Undefined callable 'x' in type Obj.

// You can bind anything, but:
print try Lol.ctor.bind([])() else error // [Expect] Type Seq does not support property-set access.
