cls X { ctor {} }

print X.__name // [Expect] X
print X.ctor   // [Expect] <Fn ctor>