cls A {              // [Exit] 2
  ctor -> this.x = 1 // [ExpectError] Compile error at line 2 at 'this': Constructors can't be lambda functions.
}

