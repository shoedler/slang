cls X {
  ctor {
    this.map = { 1: 2 }
  }
}

print X().map // [Expect] {1: 2}