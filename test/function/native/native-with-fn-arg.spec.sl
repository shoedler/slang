// Native calls managed calls native etc. stuff can lead to confusing stack traces.
// Now that we track the current native, the stack trace should be more accurate.
cls A {
  fn to_str -> this.check(fn (x) { throw x })
  fn check(f) {
    [true].each(fn (x) { f(x) })
  }
}

print ({"a": A(), "b": A() }).to_str()

// [ExpectRuntimeError] Uncaught error: true
// [ExpectRuntimeError]   at line 4 in "__anon" in module "main"
// [ExpectRuntimeError]   at line 6 in "__anon" in module "main"
// [ExpectRuntimeError]   at line 6 in native "each"
// [ExpectRuntimeError]   at line 6 in "check" in module "main"
// [ExpectRuntimeError]   at line 4 in "to_str" in module "main"
// [ExpectRuntimeError]   at line 10 at the toplevel of module "main"