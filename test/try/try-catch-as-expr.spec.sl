// Print expects an expression - using try {... will compile a try-statement, not an expression.
// [Exit] 2
print try { print "hello" } catch { print "world" } // [ExpectError] Compile error at line 3 at 'print': Expecting expression.
                                                    // [ExpectError] Compile error at line 3 at '}': Expecting expression.