cls A {              // [exit] 2
  ctor -> this.x = 1 // [expect-error] Parser error at line 2 at '->': Constructors can't be lambda functions.
}                    // [expect-error]      2 |   ctor -> this.x = 1
                     // [expect-error]                 ~~