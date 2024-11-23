cls X {            // [exit] 2
  static ctor {    // [expect-error] Compile error at line 2 at 'ctor': Expecting method initializer.
    print "X.ctor" 
  }                // [expect-error] Compile error at line 4 at '}': Expecting expression.
}
