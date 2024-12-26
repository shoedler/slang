// [exit] 2
cls Foo {
  fn method() {
    print method // [expect-error] Resolver error at line 4: Undefined variable 'method'.
  }              // [expect-error]      4 |     print method
}                // [expect-error]                    ~~~~~~
 
Foo().method()