cls X {            // [exit] 2
  static ctor {    // [expect-error] Parser error at line 2 at 'ctor': Expecting method initializer.
    print "X.ctor" // [expect-error]      2 |   static ctor { 
  }                // [expect-error]                   ~~~~
}                  // [expect-error] Parser error at line 4 at '}': Expecting expression.
                   // [expect-error]      4 |   }
                   // [expect-error]            ~