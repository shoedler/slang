// [exit] 2
cls X { ctor {this.x = 0} }

print X().() // [expect-error] Parser error at line 4 at '(': Expecting property or method name after '.'.
             // [expect-error]      4 | print X().()
             // [expect-error]                    ~