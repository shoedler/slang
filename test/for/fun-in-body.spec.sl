// It's actually allowed to have an anonymous function here, but not a named one. (Function declaration)
for ;;; fn foo() {} // [ExpectError] Compile error at line 2 at 'foo': Expecting '{' or '->' before function body.
