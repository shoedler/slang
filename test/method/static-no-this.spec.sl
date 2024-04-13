cls A {
  ctor { this.y = 1 }
  static fn x -> this.y // [ExpectCompileError] Compile error at line 3 at 'this': Can't use 'this' in a static method.
}