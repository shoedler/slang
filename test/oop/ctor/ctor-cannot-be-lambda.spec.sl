cls A {              // [exit] 2
  ctor -> this.x = 1 // [expect-error] Compile error at line 2 at 'this': Constructors can't be lambda functions.
}

