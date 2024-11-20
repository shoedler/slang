cls A {
  ctor { this.y = 1 }   // [Exit] 2
  static fn x -> this.y // [ExpectError] Compile error at line 3 at 'this': Can't use 'this' in a static method.
}