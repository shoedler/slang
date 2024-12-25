// [exit] 2
cls Foo : Foo {} // [expect-error] Parser error at line 2 at 'Foo': A class can't inherit from itself.
                // [expect-error]      2 | cls Foo : Foo {}
                 // [expect-error]                    ~~~