cls A { 
  static fn x -> 1
}

cls B : A {
  static fn x -> base.x // [ExpectCompileError] Compile error at line 6 at 'base': Can't use 'base' in a static method.
}