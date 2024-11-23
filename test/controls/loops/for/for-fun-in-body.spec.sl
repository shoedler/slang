// It's actually allowed to have an anonymous function here, but not a named one. (Function declaration)
// [exit] 2
for ;;; fn foo() {} // [expect-error] Compile error at line 3 at 'foo': Expecting '{' or '->' before function body.
