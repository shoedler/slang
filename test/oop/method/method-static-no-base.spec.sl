cls A { 
  static fn x -> 1
}

cls B : A {             // [exit] 2
  static fn x -> base.x // [expect-error] Resolver error at line 6: Can't use 'base' in a static method.
}                       // [expect-error]      6 |   static fn x -> base.x
                        // [expect-error]                           ~~~~
                        // [expect-error] Resolver error at line 6: Undefined variable 'this'.
                        // [expect-error]      6 |   static fn x -> base.x
                        // [expect-error]                           ~~~~