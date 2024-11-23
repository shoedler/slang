cls Is {
  static fn true_ -> Is(
    fn (a) -> a == true ? nil : "Expected actual to be true, but got " + a.to_str() + " instead."
  )

  static fn false_ -> Is(
    fn (a) -> a == false ? nil : "Expected actual to be false, but got " + a.to_str() + " instead."
  )

  static fn nil_ -> Is(
    fn (a) -> a == nil ? nil : "Expected actual to be nil, but got " + a.to_str() + " instead."
  )

  static fn equal_to(e) -> Is(
    fn (a) -> e == a ? nil : "Expected actual to be " + e.to_str() + ", but got " + a.to_str() + " instead."
  )

  static fn not_equal_to(e) -> Is(
    fn (a) -> e != a ? nil : "Expected actual to not be " + e.to_str() + ", but it was. (actual: " + a.to_str() + ")"
  )

  ctor (test_fn) {
    this.test_fn = test_fn
  }
}

cls Does {
  static fn throw_(e) -> Does(
    fn (threw, a) {
      if !threw ret "Expected an error to be thrown with value '" + e.to_str() + "', but no error was thrown."
      else if e == a ret nil
      else ret "Expected an error to be thrown with value '" + e.to_str() + "', but threw '" + a.to_str() + "' instead."
    }
  )
  static fn not_throw -> Does(
    fn (threw, a) {
      if threw ret "Expected no error to be thrown, but an error was thrown with value '" + a.to_str() + "'."
      else ret nil
    }
  )

  ctor (test_fn) {
    this.test_fn = test_fn
  }
}


cls Assert {
  static fn that(expected, comparer) {
    if comparer is Is {
      const { test_fn } = comparer
      const res = test_fn(expected)
      if res throw res
    }
    else if comparer is Does {
      if !(expected is Fn) {
        throw "First argument must be a callable"
      }

      // Use an obj (pointer) to check if the error is the same later on
      const ref = {}
      let err = ref

      try {
        expected()
      }
      catch {
        err = error      
      }

      const did_throw = err != ref
      const { test_fn } = comparer
      const res = test_fn(did_throw, err)
      if res throw res
    }
    else
      throw "Second argument must be an instance of Is or Does"
  }
}