cls Foo {}
                   // [Exit] 2
cls Bar : (Foo) {} // [ExpectError] Compile error at line 3 at '(': Expecting base class name.