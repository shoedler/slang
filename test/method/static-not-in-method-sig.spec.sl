cls X {
  static ctor {    // [ExpectCompileError] Compile error at line 2 at 'ctor': Expecting method initializer.
    print "X.ctor" 
  }                // [ExpectCompileError] Compile error at line 4 at '}': Expecting expression.
}
