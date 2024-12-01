// [exit] 3
cls Foo {
  fn method() {
    print method // [expect-error] Uncaught error: Undefined variable 'method'.
  }              // [expect-error]      4 |     print method
}                // [expect-error]                    ~~~~~~
                 // [expect-error]   at line 4 in "method" in module "main"
                 // [expect-error]   at line 10 at the toplevel of module "main"
 
Foo().method()