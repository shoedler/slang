// [exit] 3

// You can easily override the default error message by throwing a custom error object with a `to_str` method.
// Just make sure to make a robust `to_str` method that doesn't throw any errors itself.
cls TypeErr {
  ctor (expected_type, actual_value) {
    this.expected_type = expected_type
    this.actual_value = actual_value
  }

  fn to_str {                                                     // ˅ doesn't exist in TypeErr
    ret "Expected " + this.expected_type + " but got " + typeof(this.foo) + " instead."
  }
}

const x = 1.1

if !(x is Int) throw TypeErr(Int, x) // [expect-error] Uncaught error within to_str-method of previous error value. The previous error value was of type: <TypeErr>
                                     // [expect-error] Calling its to_str-method resulted in the following uncaught error: Property 'foo' does not exist on value of type TypeErr.
                                     // [expect-error]     12 |     ret "Expected " + this.expected_type + " but got " + typeof(this.foo) + " instead."
                                     // [expect-error]                                                                               ~~~
                                     // [expect-error]   at line 12 in "to_str" in module "main"
                                     // [expect-error]   at line 18 at the toplevel of module "main"