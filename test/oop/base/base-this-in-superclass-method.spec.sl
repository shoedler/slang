cls Base {
  ctor(a) {
    this.a = a
  }
}

cls Derived : Base {
  ctor(a, b) {
    base(a)
    this.b = b
  }
}

let derived = Derived("a", "b")
print derived.a // [expect] a
print derived.b // [expect] b
