cls A {
  ctor { this.y = 1 }   // [exit] 2
  static fn x -> this.y // [expect-error] Resolver error at line 3: Can't use 'this' in a static method.
}                       // [expect-error]      3 |   static fn x -> this.y
                        // [expect-error]                           ~~~~
                        // [expect-error] Resolver error at line 3: Undefined variable 'this'.
                        // [expect-error]      3 |   static fn x -> this.y
                        // [expect-error]                           ~~~~