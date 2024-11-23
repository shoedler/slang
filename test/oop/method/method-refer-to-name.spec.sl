cls Foo {
  fn method() {
    print method // [expect-error] Uncaught error: Undefined variable 'method'.
  }              // [expect-error]      3 |     print method
}                // [expect-error]                    ~~~~~~
                 // [expect-error]   at line 3 in "method" in module "main"
                 // [expect-error]   at line 9 at the toplevel of module "main"
 
Foo().method()