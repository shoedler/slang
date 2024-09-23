cls A {
  static fn x -> "Juice"
}

cls B : A { }

print A.x()                // [Expect] Juice
// Static methods don't get inherited
print try B.x() else error // [Expect] Undefined callable 'x' in type Class.

// If you want it, you have to define it
cls C : B {
  static fn x -> A.x()
}

print C.x() // [Expect] Juice