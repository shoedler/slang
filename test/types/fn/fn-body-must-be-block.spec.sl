// [exit] 2
fn f() 123 // [expect-error] Parser error at line 2 at '123': Expecting '->' or '{' before function body.
           // [expect-error]      2 | fn f() 123
           // [expect-error]                 ~~~