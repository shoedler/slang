cls X {
  ctor {
    this.list = ["Foo"]
  }
}

print X().list // [Expect] {Seq 1 [Foo]}