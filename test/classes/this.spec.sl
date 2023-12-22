cls Nested {
  fn method {
    fn function {
      print this
    }

    function()
  }
}

Nested().method() // [Expect] [Instance of Nested]