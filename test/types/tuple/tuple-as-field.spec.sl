cls X {
  ctor {
    this.tuple = ("Foo",)
  }
}

print X().tuple // [expect] (Foo)