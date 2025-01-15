cls Foo {
  ctor {         // [exit] 2
    ret "result" // [expect-error] Resolver error at line 3: Can't return a value from a constructor.
  }              // [expect-error]      3 |     ret "result"
}                // [expect-error]              ~~~~~~~~~~~~