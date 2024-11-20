cls Outer {
  fn method {
    print this // [Expect] <Instance of Outer>

    fn f() {
      print this // [Expect] <Instance of Outer>

      cls Inner {
        fn method {
          print this // [Expect] <Instance of Inner>
        }
      }

      Inner().method()
    }
    f()
  }
}

Outer().method()
