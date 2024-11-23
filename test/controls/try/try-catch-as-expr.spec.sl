// Print expects an expression - using try {... will compile a try-statement, not an expression.
// [exit] 2
print try { print "hello" } catch { print "world" } // [expect-error] Compile error at line 3 at 'print': Expecting expression.
                                                    // [expect-error] Compile error at line 3 at '}': Expecting expression.