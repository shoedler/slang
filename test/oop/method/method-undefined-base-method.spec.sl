// [exit] 3
cls Base { }
 
cls Derived : Base { 
  ctor {
    base.foo() // [expect-error] Uncaught error: Undefined callable 'foo' in type Base or any of its parent classes.
  }            // [expect-error]      6 |     base.foo()
}              // [expect-error]                   ~~~~~
               // [expect-error]   at line 6 in "ctor" in module "main"
               // [expect-error]   at line 12 at the toplevel of module "main"

Derived()