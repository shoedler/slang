cls Foo {
  fn method() {
    print method // [ExpectRuntimeError] Undefined variable 'method'.
  }              // [ExpectRuntimeError] [line 3] in fn "method"
}                // [ExpectRuntimeError] [line 7] in fn toplevel
 
Foo().method()
