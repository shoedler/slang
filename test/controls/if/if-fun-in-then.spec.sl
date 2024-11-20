// This would be allowed if it were an anonymous function, but since it is a named function, it is not allowed.
// [Exit] 2
if true fn foo() {} // [ExpectError] Compile error at line 3 at 'foo': Expecting '{' or '->' before function body.
// Now as anonymous:
if true fn() {} // ok