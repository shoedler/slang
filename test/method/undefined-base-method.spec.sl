cls Base { }
 
cls Derived : Base { 
  ctor {
    base.foo() // [ExpectRuntimeError] Uncaught error: Undefined method 'foo' in 'Base' or any of its parent classes
  }            // [ExpectRuntimeError]   at line 5 in "ctor" in module "main"
}              // [ExpectRuntimeError]   at line 9 at the toplevel of module "main"

Derived()