cls Base { }
 
cls Derived : Base { 
  ctor {
    base.foo() // [ExpectError] Uncaught error: Undefined callable 'foo' in type Base or any of its parent classes.
  }            // [ExpectError]      5 |     base.foo()
}              // [ExpectError]                   ~~~~~
               // [ExpectError]   at line 5 in "ctor" in module "main"
               // [ExpectError]   at line 11 at the toplevel of module "main"

Derived()