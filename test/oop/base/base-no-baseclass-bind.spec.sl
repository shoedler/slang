cls Base {
  fn foo() {            // [exit] 2
    base.does_not_exist // [expect-error] Resolver error at line 3: Can't use 'base' in a class without a base class.
  }                     // [expect-error]      3 |     base.does_not_exist
}                       // [expect-error]              ~~~~

Base().foo()