cls Base { }
 
cls Derived : Base { 
  ctor {
    base.foo() // [expect-error] Uncaught error: Undefined callable 'foo' in type Base or any of its parent classes.
  }            // [expect-error]      5 |     base.foo()
}              // [expect-error]                   ~~~~~
               // [expect-error]   at line 5 in "ctor" in module "main"
               // [expect-error]   at line 11 at the toplevel of module "main"

Derived()