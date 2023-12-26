cls Foo {
  fn method() {
    print method // [ExpectRuntimeError] Undefined variable 'method'.
  }              // [ExpectRuntimeError] at line 3 in "method"
}                // [ExpectRuntimeError] at line 7 at the toplevel
 
Foo().method()
