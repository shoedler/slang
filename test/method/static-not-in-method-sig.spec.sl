cls X {            // [Exit] 2
  static ctor {    // [ExpectError] Compile error at line 2 at 'ctor': Expecting method initializer.
    print "X.ctor" 
  }                // [ExpectError] Compile error at line 4 at '}': Expecting expression.
}
