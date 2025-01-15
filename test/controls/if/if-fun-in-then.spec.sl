// This would be allowed if it were an anonymous function, but since it is a named function, it is not allowed.
// [exit] 2
if true fn foo() {} // [expect-error] Parser error at line 3 at 'foo': Expecting '->' or '{' before function body.
// Now as anonymous:// [expect-error]      3 | if true fn foo() {}
if true fn() {}     // [expect-error]                     ~~~