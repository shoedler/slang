cls A { 
  static fn x -> 1
}

cls B : A {             // [exit] 2
  static fn x -> base.x // [expect-error] Compile error at line 6 at 'base': Can't use 'base' in a static method.
}