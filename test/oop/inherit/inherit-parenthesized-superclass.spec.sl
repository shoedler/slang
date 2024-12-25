cls Foo {}
                   // [exit] 2
cls Bar : (Foo) {} // [expect-error] Parser error at line 3 at '(': Expecting base class name.
                   // [expect-error]      3 | cls Bar : (Foo) {}
                   // [expect-error]                    ~