fn caller(g) {
  g()
  // g should be a function, not nil.
  print g == nil // [Expect] false
}

fn call_caller() {
  let captured_var = "before"
  let a = "a"

  fn f() {
    // Commenting the next line out prevents the bug!
    captured_var = "after"

    // Returning anything also fixes it, even nil:
    //ret nil
  }

  caller(f)
}

call_caller()
