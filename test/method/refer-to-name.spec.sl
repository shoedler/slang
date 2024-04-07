cls Foo {
  fn method() {
    print method // [ExpectRuntimeError] Uncaught error: Undefined variable 'method'.
  }              // [ExpectRuntimeError] at line 3 in "method" in module "main"
}                // [ExpectRuntimeError] at line 7 at the toplevel of module "main"
 
Foo().method()
