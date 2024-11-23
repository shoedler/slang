cls Outer {
  fn method {
    print this // [expect] <Instance of Outer>

    fn f() {
      print this // [expect] <Instance of Outer>

      cls Inner {
        fn method {
          print this // [expect] <Instance of Inner>
        }
      }

      Inner().method()
    }
    f()
  }
}

Outer().method()
