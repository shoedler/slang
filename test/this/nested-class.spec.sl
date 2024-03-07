cls Outer {
  fn method {
    print this // [Expect] <Instance Outer>

    fn f() {
      print this // [Expect] <Instance Outer>

      cls Inner {
        fn method {
          print this // [Expect] <Instance Inner>
        }
      }

      Inner().method()
    }
    f()
  }
}

Outer().method()
