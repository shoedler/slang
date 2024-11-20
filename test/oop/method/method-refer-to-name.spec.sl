cls Foo {
  fn method() {
    print method // [ExpectError] Uncaught error: Undefined variable 'method'.
  }              // [ExpectError]      3 |     print method
}                // [ExpectError]                    ~~~~~~
                 // [ExpectError]   at line 3 in "method" in module "main"
                 // [ExpectError]   at line 9 at the toplevel of module "main"
 
Foo().method()