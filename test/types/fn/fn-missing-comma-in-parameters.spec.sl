// [exit] 2
fn foo(a, b c, d, e, f) {} // [expect-error] Parser error at line 2 at 'c': Expecting ')' after parameters.
                           // [expect-error]      2 | fn foo(a, b c, d, e, f) {}
                           // [expect-error]                      ~