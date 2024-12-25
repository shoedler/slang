// [exit] 2
if true "ok" else cls Foo {} // [expect-error] Parser error at line 2 at 'cls': Expecting expression.
                             // [expect-error]      2 | if true "ok" else cls Foo {}
                             // [expect-error]                            ~~~