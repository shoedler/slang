// [exit] 3

// You can easily override the default error message by throwing a custom error object with a `to_str` method.
cls TypeErr {
  ctor (expected_type, actual_value) {
    this.expected_type = expected_type
    this.actual_value = actual_value
  }

  fn to_str {
    ret "Expected " + this.expected_type.to_str() + " but got " + typeof(this.actual_value).to_str() + " instead."
  }
}

const x = 1.1

if !(x is Int) throw TypeErr(Int, x) // [expect-error] Uncaught error: Expected <Int> but got <Float> instead.
                                     // [expect-error]     17 | if !(x is Int) throw TypeErr(Int, x)
                                     // [expect-error]                         ~~~~~~~~~~~~~~~~~~~~~
                                     // [expect-error]   at line 17 at the toplevel of module "main"


