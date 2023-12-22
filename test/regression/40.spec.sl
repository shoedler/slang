fn caller(g) {
  g()
  // g should be a function, not nil.
  print g == nil // [Expect] false
}

fn callCaller() {
  let capturedVar = "before"
  let a = "a"

  fn f() {
    // Commenting the next line out prevents the bug!
    capturedVar = "after"

    // Returning anything also fixes it, even nil:
    //ret nil;
  }

  caller(f)
}

callCaller()