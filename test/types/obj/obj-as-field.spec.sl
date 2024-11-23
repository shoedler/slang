cls X {
  ctor {
    this.obj = { 1: 2 }
  }
}

print X().obj // [expect] {1: 2}