cls X { ctor {} }

print X.__name // [expect] X
print X.ctor   // [expect] <Fn ctor>