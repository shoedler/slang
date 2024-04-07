cls X {
  ctor {
    this.obj = { 1: 2 }
  }
}

print X().obj // [Expect] {1: 2}