// Print expects an expression - using try {... will compile a try-statement, not an expression.
print try { print "hello" } catch { print "world" } // [ExpectError] Compile error at line 2 at 'print': Expecting expression.
                                                    // [ExpectError] Compile error at line 2 at '}': Expecting expression.