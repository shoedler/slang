cls Foo {}
                   // [exit] 2
cls Bar : (Foo) {} // [expect-error] Compile error at line 3 at '(': Expecting base class name.