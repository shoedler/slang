cls Foo {
  ctor(a, b) {
    this.a = a
    this.b = b
  }
}

let foo = Foo(1, 2, 3, 4) // [ExpectRuntimeError] Expected 2 arguments but got 4.
                          // [ExpectRuntimeError] [line 8] in fn toplevel