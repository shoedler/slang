cls A { 
  static fn x -> 1
}

cls B : A {             // [Exit] 2
  static fn x -> base.x // [ExpectError] Compile error at line 6 at 'base': Can't use 'base' in a static method.
}