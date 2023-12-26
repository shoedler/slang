// This would be allowed if it were an anonymous function, but since it is a named function, it is not allowed.
if true fn foo() {} // [ExpectCompileError] Compile error at line 2 at 'foo': Expecting '{' before function body.
// Now as anonymous:
if true fn() {} // ok