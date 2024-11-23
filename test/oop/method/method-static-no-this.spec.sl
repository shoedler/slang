cls A {
  ctor { this.y = 1 }   // [exit] 2
  static fn x -> this.y // [expect-error] Compile error at line 3 at 'this': Can't use 'this' in a static method.
}