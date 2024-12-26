// Print expects an expression - using try {... will compile a try-statement, not an expression.
// [exit] 2
print try { print "hello" } catch { print "world" } // [expect-error] Parser error at line 3 at 'print': Expecting expression.
                                                    // [expect-error]      3 | print try { print "hello" } catch { print "world" }
                                                    // [expect-error]                      ~~~~~
                                                    // [expect-error] Parser error at line 3 at '}': Expecting expression.
                                                    // [expect-error]      3 | print try { print "hello" } catch { print "world" }
                                                    // [expect-error]                                                            ~