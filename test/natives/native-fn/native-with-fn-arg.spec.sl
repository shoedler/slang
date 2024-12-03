// [exit] 3
// Native calls managed calls native etc. stuff can lead to confusing stack traces.
// Now that we track the current native, the stack trace should be more accurate.
cls A {
  fn to_str -> this.check(fn (x) { throw x })
  fn check(f) {
    [true].each(fn (x) { f(x) })
  }
}

print ({"a": A(), "b": A() })

// [expect-error] Uncaught error: true
// [expect-error]      5 |   fn to_str -> this.check(fn (x) { throw x })
// [expect-error]                                             ~~~~~~~
// [expect-error]   at line 5 in "(anon)" in module "main"
// [expect-error]   at line 7 in "(anon)" in module "main"
// [expect-error]   at line 7 in "check" in module "main"
// [expect-error]   at line 5 in "to_str" in module "main"
// [expect-error]   at line 11 at the toplevel of module "main"