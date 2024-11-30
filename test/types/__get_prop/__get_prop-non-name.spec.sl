// [exit] 2
cls X { ctor {this.x = 0} }

print X().() // [expect-error] Compile error at line 4 at '(': Expecting property name after '.'.